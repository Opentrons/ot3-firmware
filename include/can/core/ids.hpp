/********************************************
 * This is a generated file. Do not modify.  *
 ********************************************/
#pragma once

namespace can::ids {

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
    task_info_request = 0x304,
    task_info_response = 0x305,
    instrument_info_request = 0x306,
    pipette_info_response = 0x307,
    gripper_info_response = 0x308,
    hepauv_info_response = 0x309,
    set_serial_number = 0x30a,
    get_motor_usage_request = 0x30b,
    get_motor_usage_response = 0x30c,
    stop_request = 0x0,
    error_message = 0x2,
    get_status_request = 0x1,
    get_gear_status_response = 0x4,
    get_status_response = 0x5,
    enable_motor_request = 0x6,
    disable_motor_request = 0x7,
    move_request = 0x10,
    write_eeprom = 0x201,
    read_eeprom_request = 0x202,
    read_eeprom_response = 0x203,
    add_move_request = 0x15,
    get_move_group_request = 0x16,
    get_move_group_response = 0x17,
    execute_move_group_request = 0x18,
    clear_all_move_groups_request = 0x19,
    home_request = 0x20,
    add_sensor_move_request = 0x23,
    move_completed = 0x13,
    motor_position_request = 0x12,
    motor_position_response = 0x14,
    update_motor_position_estimation_request = 0x21,
    update_motor_position_estimation_response = 0x22,
    set_motion_constraints = 0x101,
    get_motion_constraints_request = 0x102,
    get_motion_constraints_response = 0x103,
    write_motor_driver_register_request = 0x30,
    read_motor_driver_register_request = 0x31,
    read_motor_driver_register_response = 0x32,
    write_motor_current_request = 0x33,
    read_motor_current_request = 0x34,
    read_motor_current_response = 0x35,
    set_brushed_motor_vref_request = 0x40,
    set_brushed_motor_pwm_request = 0x41,
    gripper_grip_request = 0x42,
    gripper_home_request = 0x43,
    add_brushed_linear_move_request = 0x44,
    brushed_motor_conf_request = 0x45,
    brushed_motor_conf_response = 0x46,
    set_gripper_error_tolerance = 0x47,
    gripper_jaw_state_request = 0x48,
    gripper_jaw_state_response = 0x49,
    set_gripper_jaw_holdoff_request = 0x401,
    gripper_jaw_holdoff_request = 0x402,
    gripper_jaw_holdoff_response = 0x403,
    acknowledgement = 0x50,
    read_presence_sensing_voltage_request = 0x600,
    read_presence_sensing_voltage_response = 0x601,
    attached_tools_request = 0x700,
    tools_detected_notification = 0x701,
    tip_presence_notification = 0x702,
    get_tip_status_request = 0x703,
    fw_update_initiate = 0x60,
    fw_update_data = 0x61,
    fw_update_data_ack = 0x62,
    fw_update_complete = 0x63,
    fw_update_complete_ack = 0x64,
    fw_update_status_request = 0x65,
    fw_update_status_response = 0x66,
    fw_update_start_app = 0x67,
    fw_update_erase_app = 0x68,
    fw_update_erase_app_ack = 0x69,
    limit_sw_request = 0x8,
    limit_sw_response = 0x9,
    do_self_contained_tip_action_request = 0x501,
    do_self_contained_tip_action_response = 0x502,
    gear_enable_motor_request = 0x503,
    gear_disable_motor_request = 0x504,
    gear_set_current_request = 0x505,
    gear_write_motor_driver_request = 0x506,
    gear_read_motor_driver_request = 0x507,
    read_sensor_request = 0x82,
    write_sensor_request = 0x83,
    baseline_sensor_request = 0x84,
    read_sensor_response = 0x85,
    set_sensor_threshold_request = 0x86,
    set_sensor_threshold_response = 0x87,
    sensor_diagnostic_request = 0x88,
    sensor_diagnostic_response = 0x89,
    bind_sensor_output_request = 0x8a,
    bind_sensor_output_response = 0x8b,
    peripheral_status_request = 0x8c,
    peripheral_status_response = 0x8d,
    baseline_sensor_response = 0x8e,
    send_accumulated_pressure_data = 0x8f,
    set_hepa_fan_state_request = 0x90,
    get_hepa_fan_state_request = 0x91,
    get_hepa_fan_state_response = 0x92,
    set_hepa_uv_state_request = 0x93,
    get_hepa_uv_state_request = 0x94,
    get_hepa_uv_state_response = 0x95,
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
    gripper = 0x20,
    gripper_z = 0x21,
    gripper_g = 0x22,
    hepa_uv = 0x32,
    pipette_left_bootloader = 0x6f,
    pipette_right_bootloader = 0x7f,
    gantry_x_bootloader = 0x3f,
    gantry_y_bootloader = 0x4f,
    head_bootloader = 0x5f,
    gripper_bootloader = 0x2f,
    hepa_uv_bootloader = 0x3e,
};

/** Common error codes. */
enum class ErrorCode {
    ok = 0x0,
    invalid_size = 0x1,
    bad_checksum = 0x2,
    invalid_byte_count = 0x3,
    invalid_input = 0x4,
    hardware = 0x5,
    timeout = 0x6,
    estop_detected = 0x7,
    collision_detected = 0x8,
    labware_dropped = 0x9,
    estop_released = 0xa,
    motor_busy = 0xb,
    stop_requested = 0xc,
    over_pressure = 0xd,
    door_open = 0xe,
    reed_open = 0xf,
};

/** Error Severity levels. */
enum class ErrorSeverity {
    warning = 0x1,
    recoverable = 0x2,
    unrecoverable = 0x3,
};

/** Tool types detected on Head. */
enum class ToolType {
    pipette = 0x0,
    gripper = 0x1,
    nothing_attached = 0x2,
    tool_error = 0x3,
};

/** Sensor types available. */
enum class SensorType {
    tip = 0x0,
    capacitive = 0x1,
    environment = 0x2,
    pressure = 0x3,
    pressure_temperature = 0x4,
    humidity = 0x5,
    temperature = 0x6,
};

/** Sensor IDs available.

    Not to be confused with SensorType. This is the ID value that separate
    two or more of the same type of sensor within a system.
     */
enum class SensorId {
    S0 = 0x0,
    S1 = 0x1,
    UNUSED = 0x2,
};

/** Links sensor threshold triggers to pins. */
enum class SensorOutputBinding {
    none = 0x0,
    sync = 0x1,
    report = 0x2,
    max_threshold_sync = 0x4,
};

/** How a sensor's threshold should be interpreted. */
enum class SensorThresholdMode {
    absolute = 0x0,
    auto_baseline = 0x1,
};

/** Tip action types. */
enum class PipetteTipActionType {
    clamp = 0x0,
    home = 0x1,
};

/** Tip action types. */
enum class GearMotorId {
    left = 0x0,
    right = 0x1,
};

/** Flags for motor position validity. */
enum class MotorPositionFlags {
    stepper_position_ok = 0x1,
    encoder_position_ok = 0x2,
};

/** Move Stop Condition. */
enum class MoveStopCondition {
    none = 0x0,
    limit_switch = 0x1,
    sync_line = 0x2,
    encoder_position = 0x4,
    gripper_force = 0x8,
    stall = 0x10,
    ignore_stalls = 0x20,
    limit_switch_backoff = 0x40,
};

/** High-level type of pipette. */
enum class PipetteType {
    pipette_single = 0x1,
    pipette_multi = 0x2,
    pipette_96 = 0x3,
};

/** Type of motor Usage value types. */
enum class MotorUsageValueType {
    linear_motor_distance = 0x0,
    left_gear_motor_distance = 0x1,
    right_gear_motor_distance = 0x2,
    force_application_time = 0x3,
    total_error_count = 0x4,
};

}  // namespace can::ids
