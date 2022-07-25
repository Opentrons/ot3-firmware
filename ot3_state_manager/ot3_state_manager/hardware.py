"""File containing hardware dataclasses for OT3."""

from __future__ import annotations

from dataclasses import dataclass

from ot3_state_manager.measurable_states import Force, Position
from ot3_state_manager.ot3_axis import OT3Axis
from ot3_state_manager.pipette_model import PipetteModel


class OT3HardwareNames:
    """Names of hardware on OT3."""

    GANTRY_X = "Gantry X"
    GANTRY_Y = "Gantry Y"
    GRIPPER = "Gripper"
    LEFT_PIPETTE = "Left Pipette"
    RIGHT_PIPETTE = "Right Pipette"
    HEAD = "Head"


@dataclass
class GantryX:
    """Gantry X Hardware."""

    hardware_name = OT3HardwareNames.GANTRY_X
    position = Position(axis=OT3Axis.X)


@dataclass
class GantryY:
    """Gantry Y Hardware."""

    hardware_name = OT3HardwareNames.GANTRY_Y
    position = Position(axis=OT3Axis.Y)


@dataclass
class Gripper:
    """Gripper Hardware."""

    hardware_name = OT3HardwareNames.GRIPPER
    position = Position(axis=OT3Axis.G)
    jaw_force = Force()


@dataclass
class LeftPipette:
    """Left Pipette Hardware"""

    hardware_name = OT3HardwareNames.LEFT_PIPETTE
    model: PipetteModel
    position = Position(axis=OT3Axis.P_L)


@dataclass
class RightPipette:
    """Right Pipette Hardware"""

    hardware_name = OT3HardwareNames.RIGHT_PIPETTE
    model: PipetteModel
    position = Position(axis=OT3Axis.P_R)


@dataclass
class Head:
    """OT3 Head Hardware."""

    hardware_name = OT3HardwareNames.HEAD
    left_pipette_mount_position = Position(axis=OT3Axis.Z_L)
    right_pipette_mount_position = Position(axis=OT3Axis.Z_R)
    gripper_mount_position = Position(axis=OT3Axis.Z_G)
