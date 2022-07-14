/********************************************
 * This is a generated file. Do not modify.  *
 ********************************************/
#pragma once

/** Can bus arbitration id function code. */
typedef enum {
    can_functioncode_network_management = 0x0,
    can_functioncode_sync = 0x1,
    can_functioncode_error = 0x2,
    can_functioncode_command = 0x3,
    can_functioncode_status = 0x4,
    can_functioncode_parameters = 0x5,
    can_functioncode_bootloader = 0x6,
    can_functioncode_heartbeat = 0x7,
} CANFunctionCode;

/** Can bus arbitration id message id. */
typedef enum {
    can_messageid_heartbeat_request = 0x3ff,
    can_messageid_heartbeat_response = 0x3fe,
    can_messageid_device_info_request = 0x302,
    can_messageid_device_info_response = 0x303,
    can_messageid_task_info_request = 0x304,
    can_messageid_task_info_response = 0x305,
    can_messageid_instrument_info_request = 0x306,
    can_messageid_pipette_info_response = 0x307,
    can_messageid_gripper_info_response = 0x308,
    can_messageid_set_serial_number = 0x30a,
    can_messageid_stop_request = 0x0,
    can_messageid_get_status_request = 0x1,
    can_messageid_get_status_response = 0x5,
    can_messageid_enable_motor_request = 0x6,
    can_messageid_disable_motor_request = 0x7,
    can_messageid_move_request = 0x10,
    can_messageid_setup_request = 0x2,
    can_messageid_write_eeprom = 0x201,
    can_messageid_read_eeprom_request = 0x202,
    can_messageid_read_eeprom_response = 0x203,
    can_messageid_add_move_request = 0x15,
    can_messageid_get_move_group_request = 0x16,
    can_messageid_get_move_group_response = 0x17,
    can_messageid_execute_move_group_request = 0x18,
    can_messageid_clear_all_move_groups_request = 0x19,
    can_messageid_home_request = 0x20,
    can_messageid_move_completed = 0x13,
    can_messageid_encoder_position_request = 0x12,
    can_messageid_encoder_position_response = 0x14,
    can_messageid_set_motion_constraints = 0x101,
    can_messageid_get_motion_constraints_request = 0x102,
    can_messageid_get_motion_constraints_response = 0x103,
    can_messageid_write_motor_driver_register_request = 0x30,
    can_messageid_read_motor_driver_register_request = 0x31,
    can_messageid_read_motor_driver_register_response = 0x32,
    can_messageid_write_motor_current_request = 0x33,
    can_messageid_read_motor_current_request = 0x34,
    can_messageid_read_motor_current_response = 0x35,
    can_messageid_set_brushed_motor_vref_request = 0x40,
    can_messageid_set_brushed_motor_pwm_request = 0x41,
    can_messageid_gripper_grip_request = 0x42,
    can_messageid_gripper_home_request = 0x43,
    can_messageid_read_presence_sensing_voltage_request = 0x600,
    can_messageid_read_presence_sensing_voltage_response = 0x601,
    can_messageid_attached_tools_request = 0x700,
    can_messageid_tools_detected_notification = 0x701,
    can_messageid_fw_update_initiate = 0x60,
    can_messageid_fw_update_data = 0x61,
    can_messageid_fw_update_data_ack = 0x62,
    can_messageid_fw_update_complete = 0x63,
    can_messageid_fw_update_complete_ack = 0x64,
    can_messageid_fw_update_status_request = 0x65,
    can_messageid_fw_update_status_response = 0x66,
    can_messageid_fw_update_start_app = 0x67,
    can_messageid_fw_update_erase_app = 0x68,
    can_messageid_fw_update_erase_app_ack = 0x69,
    can_messageid_limit_sw_request = 0x8,
    can_messageid_limit_sw_response = 0x9,
    can_messageid_do_self_contained_tip_action_request = 0x501,
    can_messageid_do_self_contained_tip_action_response = 0x502,
    can_messageid_read_sensor_request = 0x82,
    can_messageid_write_sensor_request = 0x83,
    can_messageid_baseline_sensor_request = 0x84,
    can_messageid_read_sensor_response = 0x85,
    can_messageid_set_sensor_threshold_request = 0x86,
    can_messageid_set_sensor_threshold_response = 0x87,
    can_messageid_sensor_diagnostic_request = 0x88,
    can_messageid_sensor_diagnostic_response = 0x89,
    can_messageid_bind_sensor_output_request = 0x8a,
    can_messageid_bind_sensor_output_response = 0x8b,
    can_messageid_peripheral_status_request = 0x8c,
    can_messageid_peripheral_status_response = 0x8d,
} CANMessageId;

/** Can bus arbitration id node id. */
typedef enum {
    can_nodeid_broadcast = 0x0,
    can_nodeid_host = 0x10,
    can_nodeid_pipette_left = 0x60,
    can_nodeid_pipette_right = 0x70,
    can_nodeid_gantry_x = 0x30,
    can_nodeid_gantry_y = 0x40,
    can_nodeid_head = 0x50,
    can_nodeid_head_l = 0x51,
    can_nodeid_head_r = 0x52,
    can_nodeid_gripper = 0x20,
    can_nodeid_gripper_z = 0x21,
    can_nodeid_gripper_g = 0x22,
    can_nodeid_pipette_left_bootloader = 0x6f,
    can_nodeid_pipette_right_bootloader = 0x7f,
    can_nodeid_gantry_x_bootloader = 0x3f,
    can_nodeid_gantry_y_bootloader = 0x4f,
    can_nodeid_head_bootloader = 0x5f,
    can_nodeid_gripper_bootloader = 0x2f,
} CANNodeId;

/** Common error codes. */
typedef enum {
    can_errorcode_ok = 0x0,
    can_errorcode_invalid_size = 0x1,
    can_errorcode_bad_checksum = 0x2,
    can_errorcode_invalid_byte_count = 0x3,
    can_errorcode_invalid_input = 0x4,
    can_errorcode_hardware = 0x5,
} CANErrorCode;

/** Tool types detected on Head. */
typedef enum {
    can_tooltype_undefined_tool = 0x0,
    can_tooltype_pipette_96_chan = 0x1,
    can_tooltype_pipette_384_chan = 0x2,
    can_tooltype_pipette_single_chan = 0x3,
    can_tooltype_pipette_multi_chan = 0x4,
    can_tooltype_gripper = 0x5,
    can_tooltype_nothing_attached = 0x6,
} CANToolType;

/** Sensor types available. */
typedef enum {
    can_sensortype_tip = 0x0,
    can_sensortype_capacitive = 0x1,
    can_sensortype_humidity = 0x2,
    can_sensortype_temperature = 0x3,
    can_sensortype_pressure = 0x4,
    can_sensortype_pressure_temperature = 0x5,
} CANSensorType;

/** Sensor IDs available.
    
    Not to be confused with SensorType. This is the ID value that separate
    two or more of the same type of sensor within a system.
     */
typedef enum {
    can_sensorid_s0 = 0x0,
    can_sensorid_s1 = 0x1,
} CANSensorId;

/** Links sensor threshold triggers to pins. */
typedef enum {
    can_sensoroutputbinding_none = 0x0,
    can_sensoroutputbinding_sync = 0x1,
    can_sensoroutputbinding_report = 0x2,
} CANSensorOutputBinding;

/** How a sensor's threshold should be interpreted. */
typedef enum {
    can_sensorthresholdmode_absolute = 0x0,
    can_sensorthresholdmode_auto_baseline = 0x1,
} CANSensorThresholdMode;

/** Tip action types. */
typedef enum {
    can_pipettetipactiontype_pick_up = 0x0,
    can_pipettetipactiontype_drop = 0x1,
} CANPipetteTipActionType;

/** A bit field of the arbitration id parts. */
typedef struct {
    unsigned int function_code: 4;
    unsigned int node_id: 7;
    unsigned int originating_node_id: 7;
    unsigned int message_id: 11;
    unsigned int padding: 3;
} CANArbitrationIdParts;

