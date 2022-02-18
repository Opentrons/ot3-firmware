/********************************************
 * This is a generated file. Do not modify.  *
 ********************************************/
#pragma once

namespace can_ids {

/** Can bus arbitration id function code. */
enum class FunctionCode {
    network_management = 0x0,
    sync = 0x1,
    error = 0x2,
    command = 0x3,
    status = 0x4,
    parameters = 0x5,
    bootloader = 0x6,
    heartbeat = 0x7,
};

/** Can bus arbitration id message id. */
enum class MessageId {
    heartbeat_request = 0x3ff,
    heartbeat_response = 0x3fe,
    device_info_request = 0x302,
    device_info_response = 0x303,
    stop_request = 0x0,
    get_status_request = 0x1,
    get_status_response = 0x5,
    enable_motor_request = 0x6,
    disable_motor_request = 0x7,
    move_request = 0x10,
    setup_request = 0x2,
    write_eeprom = 0x201,
    read_eeprom_request = 0x202,
    read_eeprom_response = 0x203,
    add_move_request = 0x15,
    get_move_group_request = 0x16,
    get_move_group_response = 0x17,
    execute_move_group_request = 0x18,
    clear_all_move_groups_request = 0x19,
    move_completed = 0x13,
    set_motion_constraints = 0x101,
    get_motion_constraints_request = 0x102,
    get_motion_constraints_response = 0x103,
    write_motor_driver_register_request = 0x30,
    read_motor_driver_register_request = 0x31,
    read_motor_driver_register_response = 0x32,
    read_presence_sensing_voltage_request = 0x600,
    read_presence_sensing_voltage_response = 0x601,
    attached_tools_request = 0x700,
    tools_detected_notification = 0x701,
    fw_update_initiate = 0x60,
    fw_update_data = 0x61,
    fw_update_data_ack = 0x62,
    fw_update_complete = 0x63,
    fw_update_complete_ack = 0x64,
    fw_update_status_request = 0x65,
    fw_update_status_response = 0x66,
    limit_sw_request = 0x8,
    limit_sw_response = 0x9,
    read_sensor_request = 0x82,
    write_sensor_request = 0x83,
    baseline_sensor_request = 0x84,
    read_sensor_response = 0x85,
    set_sensor_threshold_request = 0x86,
    set_sensor_threshold_response = 0x87,
};

/** Can bus arbitration id node id. */
enum class NodeId {
    broadcast = 0x0,
    host = 0x10,
    pipette_left = 0x60,
    pipette_right = 0x70,
    gantry_x = 0x30,
    gantry_y = 0x40,
    head = 0x50,
    head_l = 0x51,
    head_r = 0x52,
    pipette_left_bootloader = 0x6f,
    pipette_right_bootloader = 0x7f,
    gantry_x_bootloader = 0x3f,
    gantry_y_bootloader = 0x4f,
    head_bootloader = 0x5f,
};

/** Common error codes. */
enum class ErrorCode {
    ok = 0x0,
    invalid_size = 0x1,
    bad_checksum = 0x2,
    invalid_byte_count = 0x3,
    invalid_input = 0x4,
    hardware = 0x5,
};

/** Tool types detected on Head. */
enum class ToolType {
    undefined_tool = 0x0,
    pipette_96_chan = 0x1,
    pipette_384_chan = 0x2,
    pipette_single_chan = 0x3,
    pipette_multi_chan = 0x4,
    gripper = 0x5,
    nothing_attached = 0x6,
};

/** Sensor types available. */
enum class SensorType {
    tip = 0x0,
    capacitive = 0x1,
    humidity = 0x2,
    temperature = 0x3,
};

}  // namespace can_ids
