"""Class for all messages exchanged between client and server.

Also, location for all message handling functions.
"""

from __future__ import annotations

from dataclasses import dataclass
from typing import Literal, Optional, Union

from opentrons.hardware_control.types import OT3Axis

from ot3_state_manager.ot3_state import OT3State
from ot3_state_manager.util import Direction, get_md5_hash


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


def parse_message(string: str) -> Union[SyncPinMessage, MoveMessage]:
    """Parse string into a Message Type"""
    sync_pin_message = SyncPinMessage.to_message(string)
    move_message = MoveMessage.to_message(string)

    if sync_pin_message is not None:
        return sync_pin_message

    if move_message is not None:
        return move_message

    raise ValueError(f'Not able to parse message: "{string}"')


def handle_message(data: bytes, ot3_state: OT3State) -> bytes:
    """Parse incoming message, react to it accordingly, and respond."""
    error_response: Optional[str] = None
    try:
        message = parse_message(data.decode())
    except ValueError as err:
        error_response = f"{err.args[0]}"
    except:  # noqa: E722
        error_response = "Unhandled Exception"
    else:
        if isinstance(message, MoveMessage):
            ot3_state.pulse(message.axis, message.direction)
            print(ot3_state.current_position)
        elif isinstance(message, SyncPinMessage):
            ot3_state.set_sync_pin(message.state)
        else:
            error_response = "Parsed to an unhandled message."

    if error_response is not None:
        response = f"ERROR: {error_response}".encode()
    else:
        response = get_md5_hash(data)

    return response
