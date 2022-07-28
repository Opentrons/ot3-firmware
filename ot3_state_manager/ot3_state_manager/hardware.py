"""File containing hardware dataclasses for OT3."""

from __future__ import annotations

from dataclasses import dataclass

from opentrons.hardware_control.types import OT3Axis

from ot3_state_manager.measurable_states import Force, Position
from ot3_state_manager.pipette_model import PipetteModel


@dataclass
class GantryX:
    """Gantry X Hardware."""

    position = Position(axis=OT3Axis.X)


@dataclass
class GantryY:
    """Gantry Y Hardware."""

    position = Position(axis=OT3Axis.Y)


@dataclass
class Gripper:
    """Gripper Hardware."""

    # Gripper z is controlled by gripper board so it is being filed under gripper
    gripper_z_position = Position(axis=OT3Axis.Z_G)
    position = Position(axis=OT3Axis.G)
    jaw_force = Force(axis=OT3Axis.G)


@dataclass
class LeftPipette:
    """Left Pipette Hardware"""

    model: PipetteModel
    position = Position(axis=OT3Axis.P_L)


@dataclass
class RightPipette:
    """Right Pipette Hardware"""

    model: PipetteModel
    position = Position(axis=OT3Axis.P_R)


@dataclass
class Head:
    """OT3 Head Hardware."""

    # Putting z control for pipettes under head because that is the board that
    # actually controls it.
    left_pipette_z_position = Position(axis=OT3Axis.Z_L)
    right_pipette_z_position = Position(axis=OT3Axis.Z_R)
