#pragma once

#include "can/core/can_writer_task.hpp"
#include "can/core/ids.hpp"
#include "common/core/bit_utils.hpp"
#include "common/core/message_queue.hpp"
#include "common/firmware/gpio.hpp"
#include "hepa-uv/core/constants.h"
#include "hepa-uv/core/led_control_task.hpp"
#include "hepa-uv/core/messages.hpp"
#include "hepa-uv/firmware/gpio_drive_hardware.hpp"
#include "hepa-uv/firmware/hepa_control_hardware.hpp"

namespace hepa_task {

const uint32_t DEFAULT_HEPA_PWM = 75;

using TaskMessage = hepa_task_messages::TaskMessage;

template <led_control_task::TaskClient LEDControlClient,
          can::message_writer_task::TaskClient CanClient>
class HepaMessageHandler {
  public:
    explicit HepaMessageHandler(
        gpio_drive_hardware::GpioDrivePins &drive_pins,
        hepa_control_hardware::HepaControlHardware &hepa_hardware,
        LEDControlClient &led_control_client, CanClient &can_client)
        : drive_pins{drive_pins},
          hepa_hardware{hepa_hardware},
          led_control_client{led_control_client},
          can_client{can_client} {
        // turn off the HEPA fan
        set_hepa_fan_state(false, 0);
    }
    HepaMessageHandler(const HepaMessageHandler &) = delete;
    HepaMessageHandler(const HepaMessageHandler &&) = delete;
    auto operator=(const HepaMessageHandler &) -> HepaMessageHandler & = delete;
    auto operator=(const HepaMessageHandler &&)
        -> HepaMessageHandler && = delete;
    ~HepaMessageHandler() {}

    void handle_message(const TaskMessage &m) {
        std::visit([this](auto o) { this->visit(o); }, m);
    }

  private:
    void visit(const std::monostate &) {}

    // Handle GPIO EXTI Interrupts here
    void visit(const GPIOInterruptChanged &m) {
        if (m.pin == drive_pins.hepa_push_button.pin) {
            hepa_fan_on = !hepa_fan_on;
            set_hepa_fan_state(hepa_fan_on, hepa_fan_pwm);
        }
        // TODO: send CAN message to host
    }

    void visit(const can::messages::SetHepaFanStateRequest &m) {
        hepa_fan_pwm = (static_cast<uint32_t>(
            std::clamp(m.duty_cycle, uint32_t(0), uint32_t(100))));
        set_hepa_fan_state(bool(m.fan_on), hepa_fan_pwm);
        can_client.send_can_message(can::ids::NodeId::host,
                                    can::messages::ack_from_request(m));
    }

    void visit(const can::messages::GetHepaFanStateRequest &m) {
        auto resp = can::messages::GetHepaFanStateResponse{
            .message_index = m.message_index,
            .duty_cycle = hepa_fan_pwm,
            .fan_on = hepa_fan_on};
        can_client.send_can_message(can::ids::NodeId::host, resp);
    }

    void set_hepa_fan_state(bool turn_on = false,
                            uint32_t duty_cycle = DEFAULT_HEPA_PWM) {
        hepa_hardware.set_hepa_fan_speed(duty_cycle);
        hepa_fan_on = (turn_on && duty_cycle > 0);
        if (hepa_fan_on) {
            gpio::set(drive_pins.hepa_on_off);
            led_control_client.send_led_control_message(
                led_control_task_messages::PushButtonLED{HEPA_BUTTON, 0, 50, 0,
                                                         0});
        } else {
            gpio::reset(drive_pins.hepa_on_off);
            led_control_client.send_led_control_message(
                led_control_task_messages::PushButtonLED{HEPA_BUTTON, 0, 0, 0,
                                                         50});
        }
    }

    uint32_t hepa_fan_pwm = DEFAULT_HEPA_PWM;
    bool hepa_fan_on = false;

    gpio_drive_hardware::GpioDrivePins &drive_pins;
    hepa_control_hardware::HepaControlHardware &hepa_hardware;
    LEDControlClient &led_control_client;
    CanClient &can_client;
};

/**
 * The task type.
 */
template <template <class> class QueueImpl>
requires MessageQueue<QueueImpl<TaskMessage>, TaskMessage>
class HepaTask {
  public:
    using Messages = TaskMessage;
    using QueueType = QueueImpl<TaskMessage>;
    HepaTask(QueueType &queue) : queue{queue} {}
    HepaTask(const HepaTask &c) = delete;
    HepaTask(const HepaTask &&c) = delete;
    auto operator=(const HepaTask &c) = delete;
    auto operator=(const HepaTask &&c) = delete;
    ~HepaTask() = default;

    /**
     * Task entry point.
     */
    template <led_control_task::TaskClient LEDControlClient,
              can::message_writer_task::TaskClient CanClient>
    [[noreturn]] void operator()(
        gpio_drive_hardware::GpioDrivePins *drive_pins,
        hepa_control_hardware::HepaControlHardware *hepa_hardware,
        LEDControlClient *led_control_client, CanClient *can_client) {
        auto handler = HepaMessageHandler{*drive_pins, *hepa_hardware,
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
    {client.send_hepa_message(m)};
};

};  // namespace hepa_task
