"""Class for all messages exchanged between client and server.

Also, location for all message handling functions.
"""

from __future__ import annotations

import struct
from abc import ABC, abstractmethod
from dataclasses import dataclass
from enum import Enum
from typing import Any, Callable, Optional

from opentrons.hardware_control.types import OT3Axis

from ot3_state_manager.ot3_state import OT3State
from ot3_state_manager.util import Direction, MoveMessageHardware, SyncPinState

MESSAGE_BYTE_LENGTH = 4
MESSAGE_ID_BYTE_LENGTH = 1
MESSAGE_CONTENT_BYTE_LENGTH = 3
STRUCT_FORMAT_STRING = f"B{MESSAGE_CONTENT_BYTE_LENGTH}s"


# mypy doesn't like that I am instantiating a dataclass based on an abstract class (ABC)
# instead of a concrete class.
@dataclass  # type: ignore[misc]
class Message(ABC):
    """Parent level class for all Message objects.

    Is abstract base class that requires both a build and to_bytes method to be defined.
    """

    @staticmethod
    def parse(message_bytes: bytes) -> Message:
        """Parse sequence of bytes into a Message object."""
        if not len(message_bytes) == MESSAGE_BYTE_LENGTH:
            raise ValueError(
                f"Message length must be {MESSAGE_BYTE_LENGTH} bytes. "
                f"Your message was {len(message_bytes)} bytes."
            )
        message_id, message_content = struct.unpack(STRUCT_FORMAT_STRING, message_bytes)
        # mypy thinks that I am instantiating a MoveMessageHardware object here.
        # I am not, I am performing a lookup as defined in the enum docs.
        # https://docs.python.org/3/library/enum.html#programmatic-access-to-enumeration-members-and-their-attributes
        return MessageID(message_id).builder_func(message_content=message_content)  # type: ignore[call-arg]

    @staticmethod
    @abstractmethod
    def build(message_content: bytes) -> Message:
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
    def build(message_content: bytes) -> MoveMessage:
        """Convert message_content into a MoveMessage object."""
        hw_id, direction_val = struct.unpack(">HB", message_content)
        # mypy thinks that I am instantiating a MoveMessageHardware object here.
        # I am not, I am performing a lookup as defined in the enum docs.
        # https://docs.python.org/3/library/enum.html#programmatic-access-to-enumeration-members-and-their-attributes
        axis = MoveMessageHardware(hw_id).axis  # type: ignore[call-arg]
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
    def build(message_content: bytes) -> SyncPinMessage:
        """Convert message_content into a SyncPinMessage object."""
        # Have to use from_bytes method because struct package won't accept a 3 byte val
        val = int.from_bytes(message_content, "big")
        state = SyncPinState.from_int(val)
        return SyncPinMessage(state=state)

    def to_bytes(self) -> bytes:
        """Convert SyncPinMessage object into a sequence of hexadecimal bytes."""
        # 3rd arg is 0 because middle two bytes are not used for SyncPinMessage
        return struct.pack(">BHB", MessageID.SYNC_PIN.message_id, 0, self.state)


class MessageID(Enum):
    """Enum class defining the relationship between the message_id byte and the corresponding Message object."""

    def __new__(cls, message_id: int, builder_func: Callable) -> MessageID:
        """Create a new MessageID object.

        Lookup value will be by message_id. So MessageID(<message_id>) can be used to
        get a move message based off of the message_id.
        """
        obj = object.__new__(cls)
        obj._value_ = message_id
        obj.message_id = message_id
        obj.builder_func = builder_func
        return obj

    def __init__(self, message_id: int, builder_func: Callable) -> None:
        """Create MessageID object."""
        self.message_id = message_id
        self.builder_func = builder_func

    MOVE = 0x00, MoveMessage.build
    SYNC_PIN = 0x01, SyncPinMessage.build


def handle_message(data: bytes, ot3_state: OT3State) -> bytes:
    """Parse incoming message, react to it accordingly, and respond."""
    error_response: Optional[str] = None
    try:
        message = Message.parse(data)
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
