"""Class for all messages exchanged between client and server.

Also, location for all message handling functions.
"""

from __future__ import annotations

from dataclasses import dataclass
from typing import Any, Dict, Literal, Optional

from opentrons.hardware_control.types import OT3Axis

from ot3_state_manager.ot3_state import OT3State
from ot3_state_manager.util import Direction


@dataclass
class Message:
    """Generic Message Type."""

    def to_bytes(self) -> bytes:
        """Returns bytes value for Message."""
        return MessageLookup.message_to_bytes(self)


@dataclass
class SyncPinMessage(Message):
    """Message for setting sync pin high or low."""

    state: Literal["HIGH", "LOW"]

    def __eq__(self, other: Any) -> bool:
        """Confirm that the 2 objects are equal"""
        return isinstance(other, SyncPinMessage) and other.state == self.state

    def __hash__(self) -> int:
        """Return hash for instance."""
        return hash(str(self))


@dataclass
class MoveMessage(Message):
    """Message to be sent between hardware simulators and OT3StateManager"""

    axis: OT3Axis
    direction: Direction

    def __eq__(self, other: Any) -> bool:
        """Confirm that the 2 objects are equal"""
        return isinstance(other, MoveMessage) and (
            self.axis == other.axis and self.direction == other.direction
        )

    def __hash__(self) -> int:
        """Return hash for instance."""
        return hash(str(self))


class MessageLookup:
    """Lookup class for mapping a message to a byte value."""

    MESSAGE_LOOKUP_DICT: Dict[bytes, Message] = {
        b"0x0000": MoveMessage(OT3Axis.X, Direction.NEGATIVE),
        b"0x0001": MoveMessage(OT3Axis.X, Direction.POSITIVE),
        b"0x0010": MoveMessage(OT3Axis.Y, Direction.NEGATIVE),
        b"0x0011": MoveMessage(OT3Axis.Y, Direction.POSITIVE),
        b"0x0020": MoveMessage(OT3Axis.Z_L, Direction.NEGATIVE),
        b"0x0021": MoveMessage(OT3Axis.Z_L, Direction.POSITIVE),
        b"0x0030": MoveMessage(OT3Axis.Z_R, Direction.NEGATIVE),
        b"0x0031": MoveMessage(OT3Axis.Z_R, Direction.POSITIVE),
        b"0x0040": MoveMessage(OT3Axis.Z_G, Direction.NEGATIVE),
        b"0x0041": MoveMessage(OT3Axis.Z_G, Direction.POSITIVE),
        b"0x0050": MoveMessage(OT3Axis.P_L, Direction.NEGATIVE),
        b"0x0051": MoveMessage(OT3Axis.P_L, Direction.POSITIVE),
        b"0x0060": MoveMessage(OT3Axis.P_R, Direction.NEGATIVE),
        b"0x0061": MoveMessage(OT3Axis.P_R, Direction.POSITIVE),
        b"0x0070": MoveMessage(OT3Axis.Q, Direction.NEGATIVE),
        b"0x0071": MoveMessage(OT3Axis.Q, Direction.POSITIVE),
        b"0x0080": MoveMessage(OT3Axis.G, Direction.NEGATIVE),
        b"0x0081": MoveMessage(OT3Axis.G, Direction.POSITIVE),
        b"0x1000": SyncPinMessage("LOW"),
        b"0x1001": SyncPinMessage("HIGH"),
    }

    REVERSE_LOOKUP_DICT: Dict[Message, bytes] = {
        val: key for key, val in MESSAGE_LOOKUP_DICT.items()
    }

    @classmethod
    def message_to_bytes(cls, message: Message) -> bytes:
        """Looks up and returns byte value for message."""
        if message not in cls.REVERSE_LOOKUP_DICT:
            raise ValueError(f"Message {str(message)} not defined.")

        return cls.REVERSE_LOOKUP_DICT[message]

    @classmethod
    def bytes_to_message(cls, byte_string: bytes) -> Message:
        """Looks up and returns message for byte value."""
        if byte_string not in cls.MESSAGE_LOOKUP_DICT:
            raise ValueError(f"Message for byte string {str(byte_string)} not defined.")

        return cls.MESSAGE_LOOKUP_DICT[byte_string]

    def __init__(self) -> None:
        """Do not try to instantiate this class."""
        raise NotImplementedError


def handle_message(data: bytes, ot3_state: OT3State) -> bytes:
    """Parse incoming message, react to it accordingly, and respond."""
    error_response: Optional[str] = None
    try:
        message = MessageLookup.bytes_to_message(data)
    except ValueError as err:
        error_response = f"{err.args[0]}"
    except:  # noqa: E722
        error_response = "Unhandled Exception"
    else:
        if isinstance(message, MoveMessage):
            ot3_state.pulse(message.axis, message.direction)
        elif isinstance(message, SyncPinMessage):
            ot3_state.set_sync_pin(message.state)
        else:
            error_response = "Parsed to an unhandled message."

    if error_response is not None:
        response = f"ERROR: {error_response}".encode()
    else:
        response = data

    return response
