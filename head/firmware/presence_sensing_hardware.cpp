#include "head/firmware/presence_sensing_hardware.hpp"

#include "common/firmware/gpio.hpp"
#include "head/core/attached_tools.hpp"

using namespace presence_sensing_driver;

PresenceSensingHardware::PresenceSensingHardware(
    gpio::PinConfig left_mount_id, gpio::PinConfig right_mount_id,
    gpio::PinConfig gripper_mount_id)
    : left_mount(left_mount_id),
      right_mount(right_mount_id),
      gripper_mount(gripper_mount_id) {}

auto PresenceSensingHardware::get_readings()
    -> attached_tools::MountPinMeasurements {
    return attached_tools::MountPinMeasurements{
        .left_present = gpio::is_set(left_mount),
        .right_present = gpio::is_set(right_mount),
        .gripper_present = gpio::is_set(gripper_mount)};
}
