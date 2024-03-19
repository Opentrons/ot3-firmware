#pragma once

#include "can/core/can_writer_task.hpp"
#include "can/core/ids.hpp"
#include "common/core/bit_utils.hpp"
#include "common/core/message_queue.hpp"
#include "hepa-uv/core/constants.h"
#include "hepa-uv/core/led_control_task.hpp"
#include "hepa-uv/core/messages.hpp"
#include "hepa-uv/firmware/gpio_drive_hardware.hpp"
#include "hepa-uv/firmware/uv_control_hardware.hpp"
#include "ot_utils/freertos/freertos_timer.hpp"

namespace uv_task {

// How long to keep the UV light on in seconds.
static constexpr uint32_t DELAY_S = 60 * 15;      // 15 minutes
static constexpr uint32_t MAX_DELAY_S = 60 * 60;  // 1hr max timeout

using TaskMessage = uv_task_messages::TaskMessage;

template <led_control_task::TaskClient LEDControlClient,
          can::message_writer_task::TaskClient CanClient>
class UVMessageHandler {
  public:
    explicit UVMessageHandler(
        gpio_drive_hardware::GpioDrivePins &drive_pins,
        uv_control_hardware::UVControlHardware &uv_hardware,
        LEDControlClient &led_control_client,
        CanClient &can_client)
        : drive_pins{drive_pins},
          uv_hardware{uv_hardware},
          led_control_client{led_control_client},
          can_client{can_client},
          _timer(
              "UVTask", [ThisPtr = this] { ThisPtr->timer_callback(); },
              DELAY_S * 1000) {
        // get current state
        uv_push_button = gpio::is_set(drive_pins.uv_push_button);
        door_closed = gpio::is_set(drive_pins.door_open);
        reed_switch_set = gpio::is_set(drive_pins.reed_switch);
        // turn off UV Ballast
        gpio::reset(drive_pins.uv_on_off);
    }
    UVMessageHandler(const UVMessageHandler &) = delete;
    UVMessageHandler(const UVMessageHandler &&) = delete;
    auto operator=(const UVMessageHandler &) -> UVMessageHandler & = delete;
    auto operator=(const UVMessageHandler &&) -> UVMessageHandler && = delete;
    ~UVMessageHandler() {}

    void handle_message(const TaskMessage &m) {
        std::visit([this](auto o) { this->visit(o); }, m);
    }

  private:
    // call back to turn off the UV ballast
    void timer_callback() {
        set_uv_light_state(false);
        uv_push_button = false;
    }

    void visit(const std::monostate &) {}

    // Handle GPIO EXTI Interrupts here
    void visit(const GPIOInterruptChanged &m) {
        if (m.pin == drive_pins.hepa_push_button.pin) {
            // ignore hepa push button presses
            return;
        }

        // update states
        if (m.pin == drive_pins.uv_push_button.pin) {
            uv_push_button = !uv_push_button;
        } else if (m.pin == drive_pins.door_open.pin) {
            door_closed = gpio::is_set(drive_pins.door_open);
        } else if (m.pin == drive_pins.reed_switch.pin) {
            reed_switch_set = gpio::is_set(drive_pins.reed_switch);
        }

        // Drive the UV light
        set_uv_light_state(uv_push_button, uv_off_timeout_s);
    }

    void visit(const can::messages::SetHepaUVStateRequest &m) {
        uv_off_timeout_s = (static_cast<uint32_t>(
            std::clamp(m.timeout_s, uint32_t(0), MAX_DELAY_S)));
        can_client.send_can_message(can::ids::NodeId::host,
                                    can::messages::ack_from_request(m));
        set_uv_light_state(bool(m.uv_light_on), uv_off_timeout_s);
    }

    void visit(const can::messages::GetHepaUVStateRequest &m) {
        auto resp = can::messages::GetHepaUVStateResponse{
            .message_index = m.message_index,
            .timeout_s = uv_off_timeout_s,
            .uv_light_on = uv_light_on,
            .remaining_time_s = (_timer.get_remaining_time() / 1000),
            .uv_voltage_mv = uv_voltage_mv};
        can_client.send_can_message(can::ids::NodeId::host, resp);
    }

    void set_uv_light_state(bool light_on, uint32_t timeout_s = DELAY_S) {
        // reset push button state if the door is opened or the reed switch is
        // not set
        if (!door_closed || !reed_switch_set) {
            if (_timer.is_running()) {
                gpio::reset(drive_pins.uv_on_off);
                _timer.stop();
                // send error message when door/reed state change while uv
                // is on
                auto resp = can::messages::ErrorMessage{
                    .message_index = 0,
                    .severity = can::ids::ErrorSeverity::unrecoverable,
                    .error_code = !door_closed ? can::ids::ErrorCode::door_open
                                               : can::ids::ErrorCode::reed_open,
                };
                can_client.send_can_message(can::ids::NodeId::host, resp);
            }
            // Set the push button LED's to user intervention (blue) when
            // attempting to turn on the uv light while the door is opened or
            // reed switch is not set.
            if (light_on) {
                led_control_client.send_led_control_message(
                    led_control_task_messages::PushButtonLED(UV_BUTTON, 0, 0,
                                                             50, 0));
            }
            uv_push_button = false;
            uv_light_on = false;
            uv_voltage_mv = 0;
            return;
        }

        // set the UV Ballast
        uv_light_on = (light_on && timeout_s > 0);
        // update the push button state
        uv_push_button = uv_light_on;
        if (uv_light_on) {
            gpio::set(drive_pins.uv_on_off);
            // Set the push button LED's to running (green)
            led_control_client.send_led_control_message(
                led_control_task_messages::PushButtonLED(UV_BUTTON, 0, 50, 0,
                                                         0));
            if (_timer.is_running()) _timer.stop();
            // Update the timer in ms
            _timer.update_period(timeout_s * 1000);
            _timer.start();
        } else {
            gpio::reset(drive_pins.uv_on_off);
            // Set the push button LED's to idle (white)
            led_control_client.send_led_control_message(
                led_control_task_messages::PushButtonLED(UV_BUTTON, 0, 0, 0,
                                                         50));
            if (_timer.is_running()) _timer.stop();
        }

        // Update the voltage usage of the uv light
        uv_voltage_mv = uv_hardware.get_uv_light_voltage();

        // TODO: send state change CAN message to host
    }

    // state tracking variables
    bool door_closed = false;
    bool reed_switch_set = false;
    bool uv_push_button = false;
    bool uv_light_on = false;
    uint32_t uv_off_timeout_s = DELAY_S;
    uint32_t uv_voltage_mv = 0;

    gpio_drive_hardware::GpioDrivePins &drive_pins;
    uv_control_hardware::UVControlHardware &uv_hardware;
    LEDControlClient &led_control_client;
    CanClient &can_client;
    ot_utils::freertos_timer::FreeRTOSTimer _timer;
};

/**
 * The task type.
 */
template <template <class> class QueueImpl>
requires MessageQueue<QueueImpl<TaskMessage>, TaskMessage>
class UVTask {
  public:
    using Messages = TaskMessage;
    using QueueType = QueueImpl<TaskMessage>;
    UVTask(QueueType &queue) : queue{queue} {}
    UVTask(const UVTask &c) = delete;
    UVTask(const UVTask &&c) = delete;
    auto operator=(const UVTask &c) = delete;
    auto operator=(const UVTask &&c) = delete;
    ~UVTask() = default;

    /**
     * Task entry point.
     */
    template <led_control_task::TaskClient LEDControlClient,
              can::message_writer_task::TaskClient CanClient>
    [[noreturn]] void operator()(gpio_drive_hardware::GpioDrivePins *drive_pins,
                                 uv_control_hardware::UVControlHardware *uv_hardware,
                                 LEDControlClient *led_control_client,
                                 CanClient *can_client) {
        auto handler =
            UVMessageHandler{*drive_pins, *uv_hardware,
                             *led_control_client, *can_client};
        TaskMessage message{};
        for (;;) {
            if (queue.try_read(&message, queue.max_delay)) {
                handler.handle_message(message);
            }
        }
    }

    [[nodiscard]] auto get_queue() const -> QueueType & { return queue; }

  private:
    QueueType &queue;
};

/**
 * Concept describing a class that can message this task.
 * @tparam Client
 */
template <typename Client>
concept TaskClient = requires(Client client, const TaskMessage &m) {
    {client.send_uv_message(m)};
};

};  // namespace uv_task
