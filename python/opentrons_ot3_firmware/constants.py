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
    presence_sensor = 0x60,



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


    connected_motors_request = 0x40,
    connected_motors_response = 0x41,
    left_motor_sense_pin_voltage_request = 0x42,
    right_motor_sense_pin_voltage_request = 0x43,
    motor_sensing_alert = 0x44,
    presence_sensing_request = 0x45,
    presence_sensing_response = 0x46
