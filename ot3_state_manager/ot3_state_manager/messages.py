"""Class for all messages exchanged between client and server.

Also, location for all message handling functions.
"""

from __future__ import annotations

import struct
from abc import ABC, abstractmethod
from dataclasses import dataclass
from enum import Enum, unique
from typing import Any, Optional, Type

from opentrons.hardware_control.types import OT3Axis

from ot3_state_manager.ot3_state import OT3State
from ot3_state_manager.util import Direction, MoveMessageHardware, SyncPinState

MESSAGE_ID_BYTE_LENGTH = 1
MESSAGE_CONTENT_BYTE_LENGTH = 3
MESSAGE_BYTE_LENGTH = MESSAGE_ID_BYTE_LENGTH + MESSAGE_CONTENT_BYTE_LENGTH
STRUCT_FORMAT_STRING = f"B{MESSAGE_CONTENT_BYTE_LENGTH}s"


class Message(ABC):
    """Parent level class for all Message objects.

    Is abstract base class that requires both a build_message and to_bytes method to be defined.
    """

    @staticmethod
    @abstractmethod
    def build_message(message_content: bytes) -> Message:
        """Parse message_content into a Message object."""
        ...

    @abstractmethod
    def to_bytes(self) -> bytes:
        """Convert Message object into a sequence of hexadecimal bytes."""
        ...


@dataclass
class MoveMessage(Message):
    """Message for moving OT3 axis in specified direction."""

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

    @staticmethod
    def build_message(message_content: bytes) -> MoveMessage:
        """Convert message_content into a MoveMessage object."""
        hw_id, direction_val = struct.unpack(">HB", message_content)
        axis = MoveMessageHardware.from_id(hw_id).axis
        direction = Direction.from_int(direction_val)
        return MoveMessage(axis=axis, direction=direction)

    def to_bytes(self) -> bytes:
        """Convert MoveMessage object into a sequence of hexadecimal bytes."""
        hw_id = MoveMessageHardware.from_axis(self.axis).hw_id
        return struct.pack(">BHB", MessageID.MOVE.message_id, hw_id, self.direction)


@dataclass
class SyncPinMessage(Message):
    """Message for setting sync pin high or low."""

    state: SyncPinState

    def __eq__(self, other: Any) -> bool:
        """Confirm that the 2 objects are equal"""
        return isinstance(other, SyncPinMessage) and other.state == self.state

    def __hash__(self) -> int:
        """Return hash for instance."""
        return hash(str(self))

    @staticmethod
    def build_message(message_content: bytes) -> SyncPinMessage:
        """Convert message_content into a SyncPinMessage object."""
        assert len(message_content) == 3
        # prepend a byte to message content to make it 4 bytes long and
        # therefore parsable by struct package, without changing the resulting integer value
        message_content = b"\x00" + message_content
        val = struct.unpack(">I", message_content)[0]
        state = SyncPinState.from_int(val)
        return SyncPinMessage(state=state)

    def to_bytes(self) -> bytes:
        """Convert SyncPinMessage object into a sequence of hexadecimal bytes."""
        # 3rd arg is 0 because middle two bytes are not used for SyncPinMessage
        return struct.pack(">BHB", MessageID.SYNC_PIN.message_id, 0, self.state)


@dataclass
class GetAxisLocationMessage(Message):
    """Message to get current location of axis."""

    axis: OT3Axis

    def __eq__(self, other: Any) -> bool:
        """Confirm that the 2 objects are equal"""
        return isinstance(other, GetAxisLocationMessage) and other.axis == self.axis

    def __hash__(self) -> int:
        """Return hash for instance."""
        return hash(str(self))

    @staticmethod
    def build_message(message_content: bytes) -> GetAxisLocationMessage:
        """Convert message_content into a GetLocationMessage object."""
        hw_id, _ = struct.unpack(">HB", message_content)
        axis = MoveMessageHardware.from_id(hw_id).axis
        return GetAxisLocationMessage(axis=axis)

    def to_bytes(self) -> bytes:
        """Convert GetLocationMessage object into a sequence of hexadecimal bytes."""
        hw_id = MoveMessageHardware.from_axis(self.axis).hw_id
        return struct.pack(">BHB", MessageID.GET_LOCATION.message_id, hw_id, 0)


class GetSyncPinStateMessage(Message):
    """Message to get current location of axis."""

    def __eq__(self, other: Any) -> bool:
        """Confirm that the 2 objects are equal"""
        return isinstance(other, GetSyncPinStateMessage)

    def __hash__(self) -> int:
        """Return hash for instance."""
        return hash(str(self))

    @staticmethod
    def build_message(message_content: bytes) -> GetSyncPinStateMessage:
        """Convert message_content into a GetLocationMessage object."""
        return GetSyncPinStateMessage()

    def to_bytes(self) -> bytes:
        """Convert GetLocationMessage object into a sequence of hexadecimal bytes."""
        return struct.pack(">BHB", MessageID.GET_SYNC_PIN_STATE.message_id, 0, 0)


@unique
class MessageID(Enum):
    """Enum class defining the relationship between the message_id byte and the corresponding Message object."""

    def __init__(self, message_id: int, message_class: Type[Message]) -> None:
        """Create MessageID object."""
        self.message_id = message_id
        self.builder_func = message_class.build_message

    MOVE = 0x00, MoveMessage
    SYNC_PIN = 0x01, SyncPinMessage
    GET_LOCATION = 0x02, GetAxisLocationMessage
    GET_SYNC_PIN_STATE = 0x03, GetSyncPinStateMessage

    @classmethod
    def from_id(cls, enum_id: int) -> MessageID:
        """Get MessageID object by message_id."""
        for val in cls:
            if val.message_id == enum_id:
                return val
        else:
            raise ValueError(f"Could not find MessageID with message_id: {enum_id}.")


def _parse_message(message_bytes: bytes) -> Message:
    """Parse sequence of bytes into a Message object."""
    if not len(message_bytes) == MESSAGE_BYTE_LENGTH:
        raise ValueError(
            f"Message length must be {MESSAGE_BYTE_LENGTH} bytes. "
            f"Your message was {len(message_bytes)} bytes."
        )
    message_id, message_content = struct.unpack(STRUCT_FORMAT_STRING, message_bytes)
    return MessageID.from_id(message_id).builder_func(message_content=message_content)


def handle_message(data: bytes, ot3_state: OT3State) -> bytes:
    """Function to handle incoming message, react to it accordingly, and respond."""
    error_response: Optional[str] = None
    response = b""
    try:
        message = _parse_message(data)
    except ValueError as err:
        error_response = f"{err.args[0]}"
    except:  # noqa: E722
        error_response = "Unhandled Exception"
    else:
        if isinstance(message, MoveMessage):
            ot3_state.pulse(message.axis, message.direction)
        elif isinstance(message, SyncPinMessage):
            ot3_state.set_sync_pin(message.state)
        elif isinstance(message, GetAxisLocationMessage):
            loc = ot3_state.axis_current_position(message.axis)
            response = str(loc).encode()
        elif isinstance(message, GetSyncPinStateMessage):
            state = 1 if ot3_state.get_sync_pin_state() else 0
            response = str(state).encode()

        else:
            error_response = "Parsed to an unhandled message."

    if error_response is not None:
        response = f"ERROR: {error_response}".encode()

    if len(response) == 0:
        response = data

    return response
