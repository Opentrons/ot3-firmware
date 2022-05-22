#pragma once

#include "motor-control/core/motor_hardware_interface.hpp"

namespace encoder_handler{

// Create a class for this logic
// setup a bunch of tests for this

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

        auto get_encoder_pulses(){
                /* This function fixes the overflow issue for each motor.
            Check whether the UIEF interrupt bit in the Status register gets
            triggered. If it gets triggered we save the previous state of the
            triggered flag and clear the status register interrupt bit.
            */
            if (hardware.get_encoder_SR_flag() == true){
                hardware.clear_encoder_SR();
                if (direction) {
                    enc_overflow_counter --;
                }
                else {
                    enc_overflow_counter ++;
                    
                }
            }
            enc_position_tracker = 
                hardware.get_encoder_pulses() + static_cast<uint32_t>(enc_overflow_counter*UINT16_MAX);
            return enc_position_tracker;
        }

        void reset_encoder_pulses() { 
            enc_position_tracker = 0x0;
            enc_overflow_counter = 0x0;
            hardware.reset_encoder_pulses();
            get_encoder_pulses(); }

        void get_direction(bool dir){
            direction = dir;
        }

    private:
        motor_hardware::StepperMotorHardwareIface& hardware;
        uint8_t enc_overflow_counter = 0x0;
        uint32_t enc_position_tracker = 0x0;
        bool direction = false;
}; 
};  // namespace encoder_handler 