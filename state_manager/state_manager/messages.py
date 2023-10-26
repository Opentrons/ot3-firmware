"""Class for all messages exchanged between client and server.

Also, location for all message handling functions.
"""

from __future__ import annotations

import struct
from abc import ABC, abstractmethod
from dataclasses import dataclass
from enum import Enum, unique
from typing import Any, Optional, Type

from opentrons.hardware_control.types import Axis

from .ot3_state import OT3State
from .util import Direction, MoveMessageHardware, SyncPinState

MESSAGE_ID_BYTE_LENGTH = 1
MESSAGE_CONTENT_BYTE_LENGTH = 3
MESSAGE_BYTE_LENGTH = MESSAGE_ID_BYTE_LENGTH + MESSAGE_CONTENT_BYTE_LENGTH
STRUCT_FORMAT_STRING = f"B{MESSAGE_CONTENT_BYTE_LENGTH}s"


@dataclass
class Response:
    """A socket response."""

    content: Optional[bytes]
    is_error: bool = False
    broadcast: bool = False

    def to_bytes(self) -> bytes:
        """Convert Response object into bytes."""
        if self.content is None:
            return b""
        message = (
            f"ERROR: {self.content.decode()}".encode()
            if self.is_error
            else self.content
        )
        return message


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

    @abstractmethod
    def handle(self, data: bytes, ot3_state: OT3State) -> Optional[Response]:
        """Parse message and return a response."""
        ...


@dataclass
class MoveMessage(Message):
    """Message for moving OT3 axis in specified direction."""

    axis: Axis
    direction: Direction

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

    def handle(self, data: bytes, ot3_state: OT3State) -> Optional[Response]:
        """Parse move message and return response."""
        ot3_state.pulse(self.axis, self.direction)
        return None


@dataclass
class SyncPinMessage(Message):
    """Message for setting sync pin high or low."""

    state: SyncPinState

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

    def handle(self, data: bytes, ot3_state: OT3State) -> Optional[Response]:
        """Parse sync pin message and return a response."""
        ot3_state.set_sync_pin(self.state)
        # Set up a broadcast response
        response = GetSyncPinStateMessage().handle(b"", ot3_state)
        response.broadcast = True
        return response


@dataclass
class GetAxisLocationMessage(Message):
    """Message to get current location of axis."""

    axis: Axis

    @staticmethod
    def build_message(message_content: bytes) -> GetAxisLocationMessage:
        """Convert message_content into a GetLocationMessage object."""
        hw_id, _ = struct.unpack(">HB", message_content)
        axis = MoveMessageHardware.from_id(hw_id).axis
        return GetAxisLocationMessage(axis=axis)

    def to_bytes(self) -> bytes:
        """Convert GetLocationMessage object into a sequence of hexadecimal bytes."""
        hw_id = MoveMessageHardware.from_axis(self.axis).hw_id
        return struct.pack(">BHB", MessageID.GET_AXIS_LOCATION.message_id, hw_id, 0)

    def handle(self, data: bytes, ot3_state: OT3State) -> Response:
        """Parse get axis location message and return a response."""
        loc = str(ot3_state.axis_current_position(self.axis))
        hw_id = MoveMessageHardware.from_axis(self.axis).hw_id
        message = (
            struct.pack(">BH", MessageID.GET_AXIS_LOCATION.message_id, hw_id)
            + loc.encode()
        )
        return Response(content=message, is_error=False)


class GetSyncPinStateMessage(Message):
    """Message to get current location of axis."""

    # Need this __eq__ function because the __dict__ method of this object
    # doesn't have any attributes, so it resorts to seeing if the objects
    # reside in the same place in memory, which will basically always fail.
    # Instead, just check if the same class. This comparison works just fine
    # because GetSyncPinStateMessage does have any attributes that can change
    # value.
    def __eq__(self, other: Any) -> bool:
        """Confirm that the 2 objects are equal"""
        return isinstance(other, GetSyncPinStateMessage)

    @staticmethod
    def build_message(message_content: bytes) -> GetSyncPinStateMessage:
        """Convert message_content into a GetSyncPinStateMessage object."""
        return GetSyncPinStateMessage()

    def to_bytes(self) -> bytes:
        """Convert GetSyncPinStateMessage object into a sequence of hexadecimal bytes."""
        return struct.pack(">BHB", MessageID.GET_SYNC_PIN_STATE.message_id, 0, 0)

    def handle(self, data: bytes, ot3_state: OT3State) -> Response:
        """Parse get sync pin state message and return a response."""
        value = b"1" if ot3_state.get_sync_pin_state() else b"0"
        return Response(
            content=struct.pack(">B", MessageID.GET_SYNC_PIN_STATE.message_id) + value,
            is_error=False,
        )


@unique
class MessageID(Enum):
    """Enum class defining the relationship between the message_id byte and the corresponding Message object."""

    def __init__(self, message_id: int, message_class: Type[Message]) -> None:
        """Create MessageID object."""
        self.message_id = message_id
        self.builder_func = message_class.build_message

    MOVE = 0x00, MoveMessage
    SYNC_PIN = 0x01, SyncPinMessage
    GET_AXIS_LOCATION = 0x02, GetAxisLocationMessage
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


def handle_message(data: bytes, ot3_state: OT3State) -> Optional[Response]:
    """Function to handle incoming message, react to it accordingly, and respond."""
    response: Optional[Response]
    try:
        message = _parse_message(data)
    except ValueError as err:
        response = Response(content=None, is_error=True)
        response.content = f"{err.args[0]}".encode()
    except:  # noqa: E722
        response = Response(content=None, is_error=True)
        response.content = b"Unhandled Exception"
    else:
        if issubclass(message.__class__, Message):
            response = message.handle(data, ot3_state)
        else:
            response = Response(content=None, is_error=False)
            response.content = "Parsed to an unhandled message."
            response.is_error = True

    return response
