"""File containing hardware dataclasses for OT3."""

from __future__ import annotations

from dataclasses import dataclass

from opentrons.hardware_control.types import OT3Axis

from ot3_state_manager.measurable_states import Force, Position
from ot3_state_manager.pipette_model import PipetteModel


class GantryX:
    """Gantry X Hardware."""
    def __init__(self) -> None:
        self.position = Position(axis=OT3Axis.X)


class GantryY:
    """Gantry Y Hardware."""
    def __init__(self) -> None:
        self.position = Position(axis=OT3Axis.Y)


class Gripper:
    """Gripper Hardware."""

    # Gripper z is controlled by gripper board so it is being filed under gripper
    def __init__(self) -> None:
        self.gripper_z_position = Position(axis=OT3Axis.Z_G)
        self.position = Position(axis=OT3Axis.G)
        self.jaw_force = Force(axis=OT3Axis.G)


class LeftPipette:
    """Left Pipette Hardware"""

    def __init__(self, model: PipetteModel) -> None:
        self.model = model
        self.position = Position(axis=OT3Axis.P_L)


class RightPipette:
    """Right Pipette Hardware"""

    def __init__(self, model: PipetteModel) -> None:
        self.model = model
        self.position = Position(axis=OT3Axis.P_R)


class Head:
    """OT3 Head Hardware."""
    def __init__(self) -> None:
        # Putting z control for pipettes under head because that is the board that
        # actually controls it.
        self.left_pipette_z_position = Position(axis=OT3Axis.Z_L)
        self.right_pipette_z_position = Position(axis=OT3Axis.Z_R)


@dataclass
class SyncPin:
    sync_pin: bool = False

    def set_sync_pin_high(self):
        self.sync_pin = True

    def set_sync_pin_low(self):
        self.sync_pin = False
