#include "motor-control/core/types.hpp"

auto MotorPositionStatus::set_flag(MotorPositionStatus::Flags flag) -> void {
    backing.fetch_or(static_cast<uint8_t>(flag));
}

auto MotorPositionStatus::clear_flag(MotorPositionStatus::Flags flag) -> void {
    backing.fetch_and(~static_cast<uint8_t>(flag));
}

[[nodiscard]] auto MotorPositionStatus::check_flag(
    MotorPositionStatus::Flags flag) const -> bool {
    return (backing.load() & static_cast<uint8_t>(flag)) ==
           static_cast<uint8_t>(flag);
}

[[nodiscard]] auto MotorPositionStatus::get_flags() const -> uint8_t {
    return backing.load();
}