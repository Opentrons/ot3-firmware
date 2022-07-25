"""File containing hardware dataclasses for OT3."""

from __future__ import annotations

from abc import ABC
from dataclasses import dataclass
from enum import Enum

from ot3_state_manager.measurable_states import Force, Position
from ot3_state_manager.ot3_axis import OT3Axis
from ot3_state_manager.pipette_model import PipetteModel


class UpdatablePositions(Enum):
    """Represents positions that can be changed on the OT3"""

    GANTRY_X = "Gantry X"
    GANTRY_Y = "Gantry Y"
    GRIPPER_EXTEND = "Gripper Extend"
    GRIPPER_MOUNT = "Gripper Mount"
    LEFT_PIPETTE_MOUNT = "Left Pipette Mount"
    RIGHT_PIPETTE_MOUNT = "Right Pipette Mount"
    LEFT_PIPETTE_PLUNGER = "Left Pipette Plunger"
    RIGHT_PIPETTE_PLUNGER = "Right Pipette Plunger"

    @staticmethod
    def pos_to_axis(pos: UpdatablePositions) -> str:
        """Returns axis for passed UpdatablePosition object."""
        pos_dict = {
            UpdatablePositions.GANTRY_X: OT3Axis.X.name,
            UpdatablePositions.GANTRY_Y: OT3Axis.Y.name,
            UpdatablePositions.GRIPPER_EXTEND: OT3Axis.G.name,
            UpdatablePositions.GRIPPER_MOUNT: OT3Axis.Z_G.name,
            UpdatablePositions.LEFT_PIPETTE_MOUNT: OT3Axis.Z_L.name,
            UpdatablePositions.RIGHT_PIPETTE_MOUNT: OT3Axis.Z_R.name,
            UpdatablePositions.LEFT_PIPETTE_PLUNGER: OT3Axis.P_L.name,
            UpdatablePositions.RIGHT_PIPETTE_PLUNGER: OT3Axis.P_R.name,
        }

        try:
            axis = pos_dict[pos]
        except KeyError:
            raise Exception(f"Hardware {pos.name} does not exist")
        return axis


class Hardware(ABC):
    """ABC for all OT3 hardware."""

    hardware_name: str
    position: Position


@dataclass
class GantryX(Hardware):
    """Gantry X Hardware."""

    hardware_name = "Gantry X"
    position = Position(axis=OT3Axis.X)


@dataclass
class GantryY(Hardware):
    """Gantry Y Hardware."""

    hardware_name = "Gantry Y"
    position = Position(axis=OT3Axis.Y)


@dataclass
class Gripper(Hardware):
    """Gripper Hardware."""

    hardware_name = "Gripper"
    position = Position(axis=OT3Axis.G)
    jaw_force = Force()


@dataclass
class LeftPipette(Hardware):
    """Left Pipette Hardware"""

    hardware_name = "Left Pipette"
    model: PipetteModel
    position = Position(axis=OT3Axis.P_L)


@dataclass
class RightPipette(Hardware):
    """Right Pipette Hardware"""

    hardware_name = "Right Pipette"
    model: PipetteModel
    position = Position(axis=OT3Axis.P_R)


@dataclass
class Head(Hardware):
    """OT3 Head Hardware."""

    hardware_name = "Head"
    left_pipette_mount_position = Position(axis=OT3Axis.Z_L)
    right_pipette_mount_position = Position(axis=OT3Axis.Z_R)
    gripper_mount_position = Position(axis=OT3Axis.Z_G)
