"""Functions and classes that are shared across the package."""

from __future__ import annotations

import enum
import hashlib
from dataclasses import dataclass
from typing import Literal, Optional, Union

from opentrons.hardware_control.types import OT3Axis


class Direction(enum.Enum):
    """Class representing direction to move axis in."""

    POSITIVE = "+"
    NEGATIVE = "-"

    @classmethod
    def from_symbol(cls, symbol: str) -> "Direction":
        """Parse plus or minus symbol into direction."""
        if symbol == "+":
            return cls.POSITIVE
        elif symbol == "-":
            return cls.NEGATIVE
        else:
            raise ValueError(
                f'Symbol "{symbol}" is not valid.' f' Valid symbols are "+" and "-"'
            )


@dataclass
class Message:
    """Generic Message Type."""

    @classmethod
    def to_message(cls, string: str) -> Optional[Message]:
        """Parse string into message."""
        raise NotImplementedError


@dataclass
class SyncPinMessage(Message):
    """Message for setting sync pin high or low."""

    state: Literal["HIGH", "LOW"]

    @classmethod
    def to_message(cls, string: str) -> Optional[SyncPinMessage]:
        """Parse string into SyncPinMessage or None."""
        if string.startswith("SYNC_PIN"):
            split_string = string.split(" ")
            if len(split_string) == 2:
                sync_pin_state = split_string[1]
                if sync_pin_state == "HIGH":
                    return cls(state="HIGH")
                elif sync_pin_state == "LOW":
                    return cls(state="LOW")

        return None


@dataclass
class MoveMessage(Message):
    """Message to be sent between hardware simulators and OT3StateManager"""

    axis: OT3Axis
    direction: Direction

    @classmethod
    def to_message(cls, string: str) -> Optional[MoveMessage]:
        """Parse string to Message or None."""
        split_string = string.split(" ")

        if len(split_string) == 2:
            try:
                axis = OT3Axis[split_string[1]]
            except KeyError:
                axis = None

            try:
                direction = Direction.from_symbol(split_string[0])
            except ValueError:
                direction = None

            if axis is not None and direction is not None:
                return cls(axis=axis, direction=direction)

        return None


def get_md5_hash(data: bytes) -> bytes:
    """Convert bytes into a md5 hash."""
    return hashlib.md5(data).hexdigest().encode()


def parse_message(string: str) -> Union[SyncPinMessage, MoveMessage]:
    """Parse string into a Message Type"""
    sync_pin_message = SyncPinMessage.to_message(string)
    move_message = MoveMessage.to_message(string)

    if sync_pin_message is not None:
        return sync_pin_message

    if move_message is not None:
        return move_message

    raise ValueError(f'Not able to parse message: "{string}"')
