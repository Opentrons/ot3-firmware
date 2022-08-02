from __future__ import annotations
import enum
import hashlib
from dataclasses import dataclass

from opentrons.hardware_control.types import OT3Axis


class Direction(enum.Enum):
    """Class representing direction to move axis in."""
    POSITIVE = "+"
    NEGATIVE = "-"

    def __str__(self) -> str:
        return self.name

    @classmethod
    def from_symbol(cls, symbol: str) -> "Direction":
        """Parse plus or minus symbol into direction."""
        if symbol == "+":
            return cls.POSITIVE
        elif symbol == "-":
            return cls.NEGATIVE
        else:
            raise ValueError(
                f"Symbol \"{symbol}\" is not valid."
                f" Valid symbols are \"+\" and \"-\""
            )


@dataclass
class Message:
    """Message to be sent between hardware simulators and OT3StateManager"""
    axis: OT3Axis
    direction: Direction

    @staticmethod
    def to_message(string: str) -> Message:
        """Parse string to Message."""
        split_string = string.split(" ")
        if not len(split_string) == 2:
            raise ValueError(
                "Bad Message Format. Expects \"<axis> <direction>\". "
                f"You passed \"{string}\""
            )

        axis_string = split_string[1]

        try:
            axis = OT3Axis[axis_string]
        except KeyError:
            raise ValueError(
                f"Invalid Axis Passed. You passed \"{axis_string}\""
            )

        try:
            direction = Direction.from_symbol(split_string[0])
        except ValueError:
            raise

        return Message(
            axis=axis,
            direction=direction
        )


def get_md5_hash(data: bytes) -> bytes:
    return hashlib.md5(data).hexdigest().encode()
