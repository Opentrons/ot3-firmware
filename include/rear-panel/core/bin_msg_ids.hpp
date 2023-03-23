/********************************************
 * This is a generated file. Do not modify.  *
 ********************************************/
#pragma once

#include <cstdint>

namespace rearpanel::ids {

/** USB Binary message ID. */
enum class BinaryMessageId : uint16_t {
    echo = 0x0,
    ack = 0x1,
    ack_failed = 0x2,
    device_info_request = 0x3,
    device_info_response = 0x4,
    enter_bootloader_request = 0x5,
    enter_bootloader_response = 0x6,
    engage_estop = 0x7,
    release_estop = 0x8,
    engage_nsync_out = 0x9,
    release_nsync_out = 0xa,
    estop_state_change = 0xb,
    estop_button_detection_change = 0xc,
    door_switch_state_request = 0xd,
    door_switch_state_info = 0xe,
};

/** The types of transitons that the lights can perform. */
enum class LightTransitionType : uint8_t {
    linear = 0x0,
    sinusoid = 0x1,
    instant = 0x2,
};

/** Whether an action is looping or runs one single time. */
enum class LightAnimationType : uint8_t {
    looping = 0x0,
    single_shot = 0x1,
};

}  // namespace rearpanel::ids
