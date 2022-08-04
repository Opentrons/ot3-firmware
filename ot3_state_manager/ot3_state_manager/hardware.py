"""File containing hardware dataclasses for OT3."""

from __future__ import annotations

from dataclasses import dataclass, field

from opentrons.hardware_control.types import OT3Axis

from ot3_state_manager.measurable_states import Force, Position
from ot3_state_manager.pipette_model import PipetteModel


@dataclass
class GantryX:
    """Gantry X Hardware."""

    position: Position = field(default_factory=lambda: Position(axis=OT3Axis.X))


@dataclass
class GantryY:
    """Gantry Y Hardware."""

    position: Position = field(default_factory=lambda: Position(axis=OT3Axis.Y))


@dataclass
class Gripper:
    """Gripper Hardware."""

    # Gripper z is controlled by gripper board so it is being filed under gripper
    gripper_z_position: Position = field(
        default_factory=lambda: Position(axis=OT3Axis.Z_G)
    )
    position: Position = field(default_factory=lambda: Position(axis=OT3Axis.G))
    jaw_force: Force = field(default_factory=lambda: Force(axis=OT3Axis.G))


@dataclass
class LeftPipette:
    """Left Pipette Hardware."""

    model: PipetteModel
    position: Position = field(default_factory=lambda: Position(axis=OT3Axis.P_L))


@dataclass
class RightPipette:
    """Right Pipette Hardware."""

    model: PipetteModel
    position: Position = field(default_factory=lambda: Position(axis=OT3Axis.P_R))


@dataclass
class Head:
    """OT3 Head Hardware."""

    left_pipette_z_position: Position = field(
        default_factory=lambda: Position(axis=OT3Axis.Z_L)
    )
    right_pipette_z_position: Position = field(
        default_factory=lambda: Position(axis=OT3Axis.Z_R)
    )


@dataclass
class SyncPin:
    """OT3 Sync Pin."""

    sync_pin: bool = field(default_factory=lambda: False)

    def set_sync_pin_high(self) -> None:
        """Sets sync pin high."""
        self.sync_pin = True

    def set_sync_pin_low(self) -> None:
        """Sets sync pin low."""
        self.sync_pin = False
