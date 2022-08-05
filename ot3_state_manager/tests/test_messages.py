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
        (b"0x0000", MoveMessage(OT3Axis.X, Direction.NEGATIVE)),
        (b"0x0001", MoveMessage(OT3Axis.X, Direction.POSITIVE)),
        (b"0x0010", MoveMessage(OT3Axis.Y, Direction.NEGATIVE)),
        (b"0x0011", MoveMessage(OT3Axis.Y, Direction.POSITIVE)),
        (b"0x0020", MoveMessage(OT3Axis.Z_L, Direction.NEGATIVE)),
        (b"0x0021", MoveMessage(OT3Axis.Z_L, Direction.POSITIVE)),
        (b"0x0030", MoveMessage(OT3Axis.Z_R, Direction.NEGATIVE)),
        (b"0x0031", MoveMessage(OT3Axis.Z_R, Direction.POSITIVE)),
        (b"0x0040", MoveMessage(OT3Axis.Z_G, Direction.NEGATIVE)),
        (b"0x0041", MoveMessage(OT3Axis.Z_G, Direction.POSITIVE)),
        (b"0x0050", MoveMessage(OT3Axis.P_L, Direction.NEGATIVE)),
        (b"0x0051", MoveMessage(OT3Axis.P_L, Direction.POSITIVE)),
        (b"0x0060", MoveMessage(OT3Axis.P_R, Direction.NEGATIVE)),
        (b"0x0061", MoveMessage(OT3Axis.P_R, Direction.POSITIVE)),
        (b"0x0070", MoveMessage(OT3Axis.Q, Direction.NEGATIVE)),
        (b"0x0071", MoveMessage(OT3Axis.Q, Direction.POSITIVE)),
        (b"0x0080", MoveMessage(OT3Axis.G, Direction.NEGATIVE)),
        (b"0x0081", MoveMessage(OT3Axis.G, Direction.POSITIVE)),
        (b"0x1000", SyncPinMessage("LOW")),
        (b"0x1001", SyncPinMessage("HIGH")),
    ],
)
def test_byte_string_to_message(byte_string: bytes, expected_message: Message) -> None:
    """Confirm that move messages are parsed correctly."""
    message = MessageLookup.bytes_to_message(byte_string)
    assert message == expected_message


@pytest.mark.parametrize(
    "byte_string",
    [b"0x1111", b"0x1110"],
)
def test_invalid_message_to_message(byte_string: bytes) -> None:
    """Confirm that invalid move messages do not parse."""
    with pytest.raises(ValueError) as err:
        MessageLookup.bytes_to_message(byte_string)

    assert (
        err.value.args[0] == f"Message for byte string {str(byte_string)} not defined."
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
            "ERROR: Message for byte string b'A Bad Message' not defined.",
            id="bad_message_format",
        ),
        pytest.param(
            "+ F",
            "ERROR: Message for byte string b'+ F' not defined.",
            id="invalid_axis",
        ),
        pytest.param(
            "= X",
            "ERROR: Message for byte string b'= X' not defined.",
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
