"""Constants for can bus."""
from enum import Enum


class NodeId(int, Enum):
    """Can bus arbitration id node id."""

    broadcast = 0x00
    host = 0x10
    pipette = 0x20
    gantry_x = 0x30
    gantry_y = 0x40
    head = 0x50
    head_l = 0x51
    head_r = 0x52


class FunctionCode(int, Enum):
    """Can bus arbitration id function code."""

    network_management = 0x0
    sync = 0x1
    error = 0x2
    command = 0x3
    status = 0x4
    parameters = 0x5
    bootloader = 0x6
    heartbeat = 0x7


class MessageId(int, Enum):
    """Can bus arbitration id message id."""

    heartbeat_request = 0x3FF
    heartbeat_response = 0x3FE

    device_info_request = 0x302
    device_info_response = 0x303

    stop_request = 0x00

    get_status_request = 0x01
    get_status_response = 0x05

    enable_motor_request = 0x06
    disable_motor_request = 0x07

    move_request = 0x10

    setup_request = 0x02

    write_eeprom = 0x201
    read_eeprom_request = 0x202
    read_eeprom_response = 0x203

    add_move_request = 0x15
    get_move_group_request = 0x16
    get_move_group_response = 0x17
    execute_move_group_request = 0x18
    clear_all_move_groups_request = 0x19
    move_completed = 0x13

    set_motion_constraints = 0x101
    get_motion_constraints_request = 0x102
    get_motion_constraints_response = 0x103

    write_motor_driver_register_request = 0x30
    read_motor_driver_register_request = 0x31
    read_motor_driver_register_response = 0x32

    read_presence_sensing_voltage_request = 0x600
    read_presence_sensing_voltage_response = 0x601

    fw_update_initiate = 0x60
    fw_update_data = 0x61
    fw_update_data_ack = 0x62
    fw_update_complete = 0x63
    fw_update_complete_ack = 0x64

    limit_sw_request = 0x08
    limit_sw_response = 0x09


class ErrorCode(int, Enum):
    """Common error codes."""

    ok = 0x00
    invalid_size = 0x01
    bad_checksum = 0x02
    invalid_byte_count = 0x03
    invalid_input = 0x04
