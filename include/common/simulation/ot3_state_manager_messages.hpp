/********************************************
 * This is a generated file. Do not modify.  *
 ********************************************/
#pragma once

/** Enum class defining the relationship between the message_id byte and the corresponding Message object. */
enum class MessageID {
    move_message_id = 0x0,
    sync_pin_message_id = 0x1,
};

/** Enum representing a mapping of an integer hardware id to an OT3Axis object. */
enum class MoveMessageHardware {
    x_axis_hw_id = 0x0,
    y_axis_hw_id = 0x1,
    z_l_axis_hw_id = 0x2,
    z_r_axis_hw_id = 0x3,
    z_g_axis_hw_id = 0x4,
    p_l_axis_hw_id = 0x5,
    p_r_axis_hw_id = 0x6,
    g_axis_hw_id = 0x7,
    q_axis_hw_id = 0x8,
};

/** Class representing direction to move axis in. */
enum class Direction {
    POSITIVE = 0x1,
    NEGATIVE = 0x0,
};

/** Class representing sync pin state. */
enum class SyncPinState {
    HIGH = 0x1,
    LOW = 0x0,
};

