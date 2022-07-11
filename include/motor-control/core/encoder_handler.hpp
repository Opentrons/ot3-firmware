#include "motor-control/core/motor_hardware_interface.hpp"

namespace encoder_handler {

class EncoderHandler {
  public:
    EncoderHandler() = delete;
    EncoderHandler(motor_hardware::MotorHardwareIface& hardware_iface)
        : hardware(hardware_iface) {}
    ~EncoderHandler() = default;
    auto operator=(EncoderHandler&) -> EncoderHandler& = delete;
    auto operator=(EncoderHandler&&) -> EncoderHandler&& = delete;
    EncoderHandler(EncoderHandler&) = delete;
    EncoderHandler(EncoderHandler&&) = delete;

    auto get_encoder_pulses() {
        return hardware.get_encoder_pulses();
    }

    void reset() {
        hardware.reset_encoder_pulses();
    }

    void set_home_flag_triggered() { home_triggered = true; }

    void reset_home_flag() { home_triggered = false; }

  private:
    motor_hardware::MotorHardwareIface& hardware;
    bool home_triggered = false;
};
};  // namespace encoder_handler
