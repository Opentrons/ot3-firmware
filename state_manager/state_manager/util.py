"""Functions and classes that are shared across the package."""

from __future__ import annotations

import enum
from enum import Enum, unique

from opentrons.hardware_control.types import Axis


class Direction(int, enum.Enum):
    """Class representing direction to move axis in."""

    POSITIVE = 1
    NEGATIVE = 0

    @classmethod
    def from_int(cls, direction_val: int) -> Direction:
        """Parse integer into direction."""
        if direction_val not in (Direction.NEGATIVE, Direction.POSITIVE):
            raise ValueError(
                "Value for direction must be either 0 (Negative) or 1 "
                f"(Positive). You passed {direction_val}."
            )
        return Direction(direction_val)


class SyncPinState(int, enum.Enum):
    """Class representing sync pin state."""

    HIGH = 1
    LOW = 0

    @classmethod
    def from_int(cls, state_val: int) -> SyncPinState:
        """Parse integer into sync pin state."""
        if state_val not in (SyncPinState.LOW, SyncPinState.HIGH):
            raise ValueError(
                "Value for state must either be 0 (LOW) or 1 (HIGH). "
                f"You passed {state_val}."
            )
        return SyncPinState(state_val)


@unique
class MoveMessageHardware(Enum):
    """Enum representing a mapping of an integer hardware id to an Axis object."""

    def __init__(self, hw_id: int, axis: Axis) -> None:
        """Create a MoveMessageHardware object."""
        self.hw_id = hw_id
        self.axis = axis

    # hw_id, axis
    X_AXIS = 0x0000, Axis.X
    Y_AXIS = 0x0001, Axis.Y
    Z_L_AXIS = 0x0002, Axis.Z_L
    Z_R_AXIS = 0x0003, Axis.Z_R
    Z_G_AXIS = 0x0004, Axis.Z_G
    P_L_AXIS = 0x0005, Axis.P_L
    P_R_AXIS = 0x0006, Axis.P_R
    G_AXIS = 0x0007, Axis.G
    Q_AXIS = 0x0008, Axis.Q

    @classmethod
    def from_axis(cls, axis: Axis) -> MoveMessageHardware:
        """Get MoveMessageHardware connected to passed Axis object."""
        for val in cls:
            if val.axis == axis:
                return val
        else:
            raise ValueError(f"Could not find MoveMessageHardware with axis: {axis}.")

    @classmethod
    def from_id(cls, enum_id: int) -> MoveMessageHardware:
        """Get MoveMessageHardware connect to passed enum_id."""
        for val in cls:
            if val.hw_id == enum_id:
                return val
        else:
            raise ValueError(
                f"Could not find MoveMessageHardware with hw_id: {enum_id}."
            )
