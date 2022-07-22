"""File for enum class describing OT3 Axes"""

import enum


class OT3Axis(enum.Enum):
    """All the different OT3 axes"""

    X = 0  # gantry
    Y = 1
    Z_L = 2  # left pipette mount Z
    Z_R = 3  # right pipette mount Z
    Z_G = 4  # gripper mount Z
    P_L = 5  # left pipette plunger
    P_R = 6  # right pipette plunger
    Q = 7  # hi-throughput pipette tiprack grab
    G = 8  # gripper grab
