"""Test message functionality."""

import pytest
from opentrons.hardware_control.types import OT3Axis

from ot3_state_manager.messages import (
    Message,
    MessageLookup,
    MoveMessage,
    SyncPinMessage,
    handle_message,
)
from ot3_state_manager.ot3_state import OT3State
from ot3_state_manager.util import Direction


@pytest.mark.parametrize(
    "byte_string,expected_message",
    [
        (b"\x00\x00", MoveMessage(OT3Axis.X, Direction.NEGATIVE)),
        (b"\x00\x01", MoveMessage(OT3Axis.X, Direction.POSITIVE)),
        (b"\x00\x10", MoveMessage(OT3Axis.Y, Direction.NEGATIVE)),
        (b"\x00\x11", MoveMessage(OT3Axis.Y, Direction.POSITIVE)),
        (b"\x00\x20", MoveMessage(OT3Axis.Z_L, Direction.NEGATIVE)),
        (b"\x00\x21", MoveMessage(OT3Axis.Z_L, Direction.POSITIVE)),
        (b"\x00\x30", MoveMessage(OT3Axis.Z_R, Direction.NEGATIVE)),
        (b"\x00\x31", MoveMessage(OT3Axis.Z_R, Direction.POSITIVE)),
        (b"\x00\x40", MoveMessage(OT3Axis.Z_G, Direction.NEGATIVE)),
        (b"\x00\x41", MoveMessage(OT3Axis.Z_G, Direction.POSITIVE)),
        (b"\x00\x50", MoveMessage(OT3Axis.P_L, Direction.NEGATIVE)),
        (b"\x00\x51", MoveMessage(OT3Axis.P_L, Direction.POSITIVE)),
        (b"\x00\x60", MoveMessage(OT3Axis.P_R, Direction.NEGATIVE)),
        (b"\x00\x61", MoveMessage(OT3Axis.P_R, Direction.POSITIVE)),
        (b"\x00\x70", MoveMessage(OT3Axis.Q, Direction.NEGATIVE)),
        (b"\x00\x71", MoveMessage(OT3Axis.Q, Direction.POSITIVE)),
        (b"\x00\x80", MoveMessage(OT3Axis.G, Direction.NEGATIVE)),
        (b"\x00\x81", MoveMessage(OT3Axis.G, Direction.POSITIVE)),
        (b"\x10\x00", SyncPinMessage("LOW")),
        (b"\x10\x01", SyncPinMessage("HIGH")),
    ],
)
def test_byte_string_to_message(byte_string: bytes, expected_message: Message) -> None:
    """Confirm that move messages are parsed correctly."""
    message = MessageLookup.bytes_to_message(byte_string)
    assert message == expected_message


@pytest.mark.parametrize(
    "byte_string",
    [b"\x11\x11", b"\x11\x10"],
)
def test_nonexistent_message_to_message(byte_string: bytes) -> None:
    """Confirm that trying to load a nonexistent message throws an error."""
    with pytest.raises(ValueError) as err:
        MessageLookup.bytes_to_message(byte_string)

    assert (
        err.value.args[0] == f"Message for hexadecimal byte string {byte_string!r} is "
        f"not defined."
    )


@pytest.mark.parametrize(
    "message,expected_val",
    (
        pytest.param(MoveMessage(OT3Axis.X, Direction.POSITIVE), 1, id="pos_pulse"),
        pytest.param(MoveMessage(OT3Axis.X, Direction.NEGATIVE), -1, id="neg_pulse"),
    ),
)
def test_valid_handle_move_message(
    message: Message, expected_val: int, ot3_state: OT3State
) -> None:
    """Confirm that pulse messages work correctly."""
    print(message.to_bytes())
    ack = handle_message(message.to_bytes(), ot3_state)
    assert ack == message.to_bytes()
    assert ot3_state.axis_current_position(OT3Axis.X) == expected_val


def test_valid_handle_sync_pin_message(ot3_state: OT3State) -> None:
    """Confirm that pulse messages work correctly."""
    HIGH_MESSAGE = SyncPinMessage("HIGH")
    LOW_MESSAGE = SyncPinMessage("LOW")
    ack = handle_message(HIGH_MESSAGE.to_bytes(), ot3_state)
    assert ack == HIGH_MESSAGE.to_bytes()
    assert ot3_state.get_sync_pin_state()

    ack = handle_message(LOW_MESSAGE.to_bytes(), ot3_state)
    assert ack == LOW_MESSAGE.to_bytes()
    assert not ot3_state.get_sync_pin_state()


@pytest.mark.parametrize(
    "message, expected_error",
    (
        pytest.param(
            "A Bad Message",
            "ERROR: Passed message, \"b'A Bad Message'\" is not a valid hexadecimal "
            "byte string.",
            id="bad_message_format",
        ),
        pytest.param(
            "+ F",
            "ERROR: Passed message, \"b'+ F'\" is not a valid hexadecimal "
            "byte string.",
            id="invalid_axis",
        ),
        pytest.param(
            "= X",
            "ERROR: Passed message, \"b'= X'\" is not a valid hexadecimal "
            "byte string.",
            id="invalid_direction",
        ),
    ),
)
def test_invalid_handle_message(
    message: str, expected_error: str, ot3_state: OT3State
) -> None:
    """Confirm that invalid messages respond with the correct error."""
    response = handle_message(message.encode(), ot3_state)
    assert response.decode() == expected_error
    assert all([value == 0 for value in ot3_state.current_position.values()])
    assert all([value == 0 for value in ot3_state.encoder_position.values()])
