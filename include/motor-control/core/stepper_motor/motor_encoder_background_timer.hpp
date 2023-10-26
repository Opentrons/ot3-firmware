#pragma once

#include <concepts>
#include <cstdint>
#include <tuple>

#include "common/core/freertos_synchronization.hpp"
#include "motor-control/core/motor_hardware_interface.hpp"
#include "ot_utils/freertos/freertos_timer.hpp"

namespace motor_encoder {

template <class T>
concept InterruptHandler = requires(T handler) {
    { handler.has_active_move() } -> std::same_as<bool>;
};

template <InterruptHandler Handler, typename MotorHardware>
requires std::is_base_of_v<motor_hardware::MotorHardwareIface, MotorHardware>
class BackgroundTimer {
  public:
    BackgroundTimer(Handler &interrupt_handler, MotorHardware &motor_hardware)
        : _interrupt_handler(interrupt_handler),
          _motor_hardware(motor_hardware),
          _timer(
              "encoder_timer", [this]() { this->callback(); }, PERIOD_MS) {}

    auto start() -> void { _timer.start(); }

    auto stop() -> void { _timer.stop(); }

    auto callback() -> void {
        auto critical_section =
            freertos_synchronization::FreeRTOSCriticalSectionRAII();
        if (!_interrupt_handler.is_moving()) {
            // Refresh the overflow counter if nothing else is doing it
            // and update position flag if needed
            std::ignore = _interrupt_handler.check_for_stall();
        }
    }

  private:
    static constexpr uint32_t PERIOD_MS = 10;

    Handler &_interrupt_handler;
    MotorHardware &_motor_hardware;
    ot_utils::freertos_timer::FreeRTOSTimer _timer;
};

}  // namespace motor_encoder
