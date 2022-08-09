"""Functions and classes that are shared across the package."""

from __future__ import annotations

import enum
from enum import Enum, unique

from opentrons.hardware_control.types import OT3Axis


class Direction(int, enum.Enum):
    """Class representing direction to move axis in."""

    POSITIVE = 1
    NEGATIVE = 0

    @classmethod
    def from_int(cls, direction_val: int) -> Direction:
        """Parse integer into direction."""
        if direction_val == Direction.NEGATIVE:
            direction = Direction.NEGATIVE
        elif direction_val == Direction.POSITIVE:
            direction = Direction.POSITIVE
        else:
            raise ValueError(
                "Value for direction must be either 0 (Negative) or 1 "
                f"(Positive). You passed {direction_val}."
            )
        return direction


class SyncPinState(int, enum.Enum):
    """Class representing sync pin state."""

    HIGH = 1
    LOW = 0

    @classmethod
    def from_int(cls, state_val: int) -> SyncPinState:
        """Parse integer into sync pin state."""
        if state_val == 0:
            state = SyncPinState.LOW
        elif state_val == 1:
            state = SyncPinState.HIGH
        else:
            raise ValueError(
                "Value for state must either be 0 (LOW) or 1 (HIGH). "
                f"You passed {state_val}."
            )

        return state


@unique
class MoveMessageHardware(Enum):
    """Enum representing a mapping of an integer hardware id to an OT3Axis object."""

    def __new__(cls, hw_id: int, axis: OT3Axis) -> MoveMessageHardware:
        """Create a new MoveMessageHardware object.

        Lookup value will be by hw_id. So MoveMessageHardware(<hw_id>) can be used to
        get a move message based off of the hw_id. If you want to lookup by OT3Axis use
        MoveMessageHardware.from_axis(<ot3_axis).
        """
        obj = object.__new__(cls)
        obj._value_ = hw_id
        obj.hw_id = hw_id
        obj.axis = axis
        return obj

    def __init__(self, hw_id: int, axis: OT3Axis) -> None:
        """Create a MoveMessageHardware object."""
        self.hw_id = hw_id
        self.axis = axis

    # hw_id, axis
    X_AXIS = 0x0000, OT3Axis.X
    Y_AXIS = 0x0001, OT3Axis.Y
    Z_L_AXIS = 0x0002, OT3Axis.Z_L
    Z_R_AXIS = 0x0003, OT3Axis.Z_R
    Z_G_AXIS = 0x0004, OT3Axis.Z_G
    P_L_AXIS = 0x0005, OT3Axis.P_L
    P_R_AXIS = 0x0006, OT3Axis.P_R
    G_AXIS = 0x0007, OT3Axis.G
    Q_AXIS = 0x0008, OT3Axis.Q

    @classmethod
    def from_axis(cls, axis: OT3Axis) -> MoveMessageHardware:
        """Get MoveMessageHardware connected to passed OT3Axis object."""
        for val in cls:
            if val.axis == axis:
                return val
        else:
            raise ValueError(f"Could not find MoveMessageHardware with axis: {axis}.")
