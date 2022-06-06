#include "motor-control/core/motor_hardware_interface.hpp"

namespace encoder_handler {

class EncoderHandler {
  public:
    EncoderHandler() = delete;
    EncoderHandler(motor_hardware::StepperMotorHardwareIface& hardware_iface)
        : hardware(hardware_iface) {}
    ~EncoderHandler() = default;
    auto operator=(EncoderHandler&) -> EncoderHandler& = delete;
    auto operator=(EncoderHandler&&) -> EncoderHandler&& = delete;
    EncoderHandler(EncoderHandler&) = delete;
    EncoderHandler(EncoderHandler&&) = delete;

    auto get_encoder_pulses() {
        auto extra_pulses =
            static_cast<int32_t>(enc_overflow_counter * UINT16_MAX);
        enc_position_tracker = hardware.get_encoder_pulses() + extra_pulses;
        return enc_position_tracker;
    }

    void reset() {
        enc_position_tracker = 0x0;
        enc_overflow_counter = 0x0;
        hardware.reset_encoder_pulses();
    }

    void encoder_overflow() {
        /* This function fixes the overflow issue for each motor.
        Check whether the UIEF interrupt bit in the Status register gets
        triggered. If it gets triggered we save the previous state of the
        triggered flag and clear the status register interrupt bit.
        */
        hardware.clear_encoder_SR();
        if (!enc_direction) {
            enc_overflow_counter++;
            if (home_triggered){
                enc_overflow_counter = 0x0;
                hardware.clear_encoder_SR();
                reset_home_flag();
            }
        } 
        else if (enc_overflow_counter > 0) {
            enc_overflow_counter--;   
        }
    }

    void get_enc_direction() {
        enc_direction = hardware.get_encoder_direction();
    }
    /*  Due to the sensitivity of the direction bit in the CR1 register
        These functions were created to set which direction the motor is getting
        driven.*/
    void set_positive_direction() { enc_direction = 0x0; }

    void set_negative_direction() { enc_direction = 0x1; }

    void set_home_flag_triggered() {  home_triggered = 0x1; }

    void reset_home_flag() { home_triggered = 0x0; }

  private:
    motor_hardware::StepperMotorHardwareIface& hardware;
    uint8_t enc_overflow_counter = 0;
    int32_t enc_position_tracker = 0x0;
    bool enc_direction = 0x0;
    bool home_triggered = 0x0;
};
};  // namespace encoder_handler