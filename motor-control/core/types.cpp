#include "motor-control/core/types.hpp"

__attribute__((section(".ccmram"))) auto MotorPositionStatus::set_flag(
    MotorPositionStatus::Flags flag) -> void {
    backing.fetch_or(static_cast<uint8_t>(flag));
}

__attribute__((section(".ccmram"))) auto MotorPositionStatus::clear_flag(
    MotorPositionStatus::Flags flag) -> void {
    backing.fetch_and(~static_cast<uint8_t>(flag));
}

__attribute__((section(".ccmram"))) [[nodiscard]] auto
MotorPositionStatus::check_flag(MotorPositionStatus::Flags flag) const -> bool {
    return (backing.load() & static_cast<uint8_t>(flag)) ==
           static_cast<uint8_t>(flag);
}

__attribute__((section(".ccmram"))) [[nodiscard]] auto
MotorPositionStatus::get_flags() const -> uint8_t {
    return backing.load();
}