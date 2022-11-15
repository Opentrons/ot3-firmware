#include "motor-control/core/types.hpp"

void MotorPositionStatus::set_flag(MotorPositionStatus::Flags flag) {
    backing.fetch_or(static_cast<uint8_t>(flag));
}

void MotorPositionStatus::clear_flag(MotorPositionStatus::Flags flag) {
    backing.fetch_and(~static_cast<uint8_t>(flag));
}

bool MotorPositionStatus::check_flag(MotorPositionStatus::Flags flag) {
    return (backing.load() & static_cast<uint8_t>(flag)) ==
           static_cast<uint8_t>(flag);
}

uint8_t MotorPositionStatus::get_flags() { return backing.load(); }