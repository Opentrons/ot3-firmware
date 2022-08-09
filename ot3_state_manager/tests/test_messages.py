"""Test message functionality"""

import pytest
from opentrons.hardware_control.types import OT3Axis

from ot3_state_manager.messages import (
    Message,
    MoveMessage,
    SyncPinMessage,
    handle_message,
)
from ot3_state_manager.ot3_state import OT3State
from ot3_state_manager.util import Direction, SyncPinState


@pytest.mark.parametrize(
    "message, error",
    [
        pytest.param(
            b"\x00\x00\x00\x00\xFF",
            "ERROR: Message length must be 4 bytes. Your message was 5 bytes.",
            id="MESSAGE_TOO_LONG",
        ),
        pytest.param(
            b"\x00\x00\x00",
            "ERROR: Message length must be 4 bytes. Your message was 3 bytes.",
            id="MESSAGE_TOO_SHORT",
        ),
        pytest.param(
            b"\x00\xFF\xFF\x01",
            "ERROR: Could not find MoveMessageHardware with hw_id: 65535.",
            id="MOVE_MESSAGE_INVALID_HW_ID",
        ),
        pytest.param(
            b"\x00\x00\x00\x02",
            "ERROR: Value for direction must be either 0 (Negative) or 1 (Positive). You passed 2.",
            id="MOVE_MESSAGE_INVALID_DIRECTION",
        ),
        pytest.param(
            b"\x01\x00\x00\x02",
            "ERROR: Value for state must either be 0 (LOW) or 1 (HIGH). You passed 2.",
            id="SYNC_PIN_MESSAGE_INVALID_STATE",
        ),
    ],
)
def test_bad_messages(message: bytes, error: str, ot3_state: OT3State) -> None:
    """Confirm that if too long/short of a message is passed an exception is thrown."""
    assert handle_message(message, ot3_state).decode() == error


@pytest.mark.parametrize(
    "message,expected_message",
    (
        pytest.param(
            b"\x00\x00\x00\x00",
            MoveMessage(OT3Axis.X, Direction.NEGATIVE),
            id="MOVE_X_NEGATIVE",
        ),
        pytest.param(
            b"\x00\x00\x00\x01",
            MoveMessage(OT3Axis.X, Direction.POSITIVE),
            id="MOVE_X_POSITIVE",
        ),
        pytest.param(
            b"\x00\x00\x01\x00",
            MoveMessage(OT3Axis.Y, Direction.NEGATIVE),
            id="MOVE_Y_NEGATIVE",
        ),
        pytest.param(
            b"\x00\x00\x01\x01",
            MoveMessage(OT3Axis.Y, Direction.POSITIVE),
            id="MOVE_Y_POSITIVE",
        ),
        pytest.param(
            b"\x00\x00\x02\x00",
            MoveMessage(OT3Axis.Z_L, Direction.NEGATIVE),
            id="MOVE_Z_L_NEGATIVE",
        ),
        pytest.param(
            b"\x00\x00\x02\x01",
            MoveMessage(OT3Axis.Z_L, Direction.POSITIVE),
            id="MOVE_Z_L_POSITIVE",
        ),
        pytest.param(
            b"\x00\x00\x03\x00",
            MoveMessage(OT3Axis.Z_R, Direction.NEGATIVE),
            id="MOVE_Z_R_NEGATIVE",
        ),
        pytest.param(
            b"\x00\x00\x03\x01",
            MoveMessage(OT3Axis.Z_R, Direction.POSITIVE),
            id="MOVE_Z_R_POSITIVE",
        ),
        pytest.param(
            b"\x00\x00\x04\x00",
            MoveMessage(OT3Axis.Z_G, Direction.NEGATIVE),
            id="MOVE_Z_G_NEGATIVE",
        ),
        pytest.param(
            b"\x00\x00\x04\x01",
            MoveMessage(OT3Axis.Z_G, Direction.POSITIVE),
            id="MOVE_Z_G_POSITIVE",
        ),
        pytest.param(
            b"\x00\x00\x05\x00",
            MoveMessage(OT3Axis.P_L, Direction.NEGATIVE),
            id="MOVE_P_L_NEGATIVE",
        ),
        pytest.param(
            b"\x00\x00\x05\x01",
            MoveMessage(OT3Axis.P_L, Direction.POSITIVE),
            id="MOVE_P_L_POSITIVE",
        ),
        pytest.param(
            b"\x00\x00\x06\x00",
            MoveMessage(OT3Axis.P_R, Direction.NEGATIVE),
            id="MOVE_P_R_NEGATIVE",
        ),
        pytest.param(
            b"\x00\x00\x06\x01",
            MoveMessage(OT3Axis.P_R, Direction.POSITIVE),
            id="MOVE_P_R_POSITIVE",
        ),
        pytest.param(
            b"\x00\x00\x07\x00",
            MoveMessage(OT3Axis.G, Direction.NEGATIVE),
            id="MOVE_G_NEGATIVE",
        ),
        pytest.param(
            b"\x00\x00\x07\x01",
            MoveMessage(OT3Axis.G, Direction.POSITIVE),
            id="MOVE_G_POSITIVE",
        ),
        pytest.param(
            b"\x00\x00\x08\x00",
            MoveMessage(OT3Axis.Q, Direction.NEGATIVE),
            id="MOVE_Q_NEGATIVE",
        ),
        pytest.param(
            b"\x00\x00\x08\x01",
            MoveMessage(OT3Axis.Q, Direction.POSITIVE),
            id="MOVE_Q_POSITIVE",
        ),
        pytest.param(
            b"\x01\x00\x00\x00",
            SyncPinMessage(SyncPinState.LOW),
            id="SYNC_LOW",
        ),
        pytest.param(
            b"\x01\x00\x00\x01",
            SyncPinMessage(SyncPinState.HIGH),
            id="SYNC_HIGH",
        ),
    ),
)
def test_message_parsing(message: bytes, expected_message: Message) -> None:
    """Confirm that Message.parse parses byte string into correct Message object."""
    assert Message.parse(message) == expected_message


@pytest.mark.parametrize(
    "message, expected_bytes",
    (
        pytest.param(
            MoveMessage(OT3Axis.X, Direction.NEGATIVE),
            b"\x00\x00\x00\x00",
            id="MOVE_X_NEGATIVE",
        ),
        pytest.param(
            MoveMessage(OT3Axis.X, Direction.POSITIVE),
            b"\x00\x00\x00\x01",
            id="MOVE_X_POSITIVE",
        ),
        pytest.param(
            MoveMessage(OT3Axis.Y, Direction.NEGATIVE),
            b"\x00\x00\x01\x00",
            id="MOVE_Y_NEGATIVE",
        ),
        pytest.param(
            MoveMessage(OT3Axis.Y, Direction.POSITIVE),
            b"\x00\x00\x01\x01",
            id="MOVE_Y_POSITIVE",
        ),
        pytest.param(
            MoveMessage(OT3Axis.Z_L, Direction.NEGATIVE),
            b"\x00\x00\x02\x00",
            id="MOVE_Z_L_NEGATIVE",
        ),
        pytest.param(
            MoveMessage(OT3Axis.Z_L, Direction.POSITIVE),
            b"\x00\x00\x02\x01",
            id="MOVE_Z_L_POSITIVE",
        ),
        pytest.param(
            MoveMessage(OT3Axis.Z_R, Direction.NEGATIVE),
            b"\x00\x00\x03\x00",
            id="MOVE_Z_R_NEGATIVE",
        ),
        pytest.param(
            MoveMessage(OT3Axis.Z_R, Direction.POSITIVE),
            b"\x00\x00\x03\x01",
            id="MOVE_Z_R_POSITIVE",
        ),
        pytest.param(
            MoveMessage(OT3Axis.Z_G, Direction.NEGATIVE),
            b"\x00\x00\x04\x00",
            id="MOVE_Z_G_NEGATIVE",
        ),
        pytest.param(
            MoveMessage(OT3Axis.Z_G, Direction.POSITIVE),
            b"\x00\x00\x04\x01",
            id="MOVE_Z_G_POSITIVE",
        ),
        pytest.param(
            MoveMessage(OT3Axis.P_L, Direction.NEGATIVE),
            b"\x00\x00\x05\x00",
            id="MOVE_P_L_NEGATIVE",
        ),
        pytest.param(
            MoveMessage(OT3Axis.P_L, Direction.POSITIVE),
            b"\x00\x00\x05\x01",
            id="MOVE_P_L_POSITIVE",
        ),
        pytest.param(
            MoveMessage(OT3Axis.P_R, Direction.NEGATIVE),
            b"\x00\x00\x06\x00",
            id="MOVE_P_R_NEGATIVE",
        ),
        pytest.param(
            MoveMessage(OT3Axis.P_R, Direction.POSITIVE),
            b"\x00\x00\x06\x01",
            id="MOVE_P_R_POSITIVE",
        ),
        pytest.param(
            MoveMessage(OT3Axis.G, Direction.NEGATIVE),
            b"\x00\x00\x07\x00",
            id="MOVE_G_NEGATIVE",
        ),
        pytest.param(
            MoveMessage(OT3Axis.G, Direction.POSITIVE),
            b"\x00\x00\x07\x01",
            id="MOVE_G_POSITIVE",
        ),
        pytest.param(
            MoveMessage(OT3Axis.Q, Direction.NEGATIVE),
            b"\x00\x00\x08\x00",
            id="MOVE_Q_NEGATIVE",
        ),
        pytest.param(
            MoveMessage(OT3Axis.Q, Direction.POSITIVE),
            b"\x00\x00\x08\x01",
            id="MOVE_Q_POSITIVE",
        ),
        pytest.param(
            SyncPinMessage(SyncPinState.LOW),
            b"\x01\x00\x00\x00",
            id="SYNC_LOW",
        ),
        pytest.param(
            SyncPinMessage(SyncPinState.HIGH),
            b"\x01\x00\x00\x01",
            id="SYNC_HIGH",
        ),
    ),
)
def test_convert_message_to_bytes(message: Message, expected_bytes: bytes) -> None:
    """Confirm that converting a message into bytes works correctly."""
    assert message.to_bytes() == expected_bytes


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
    HIGH_MESSAGE = SyncPinMessage(SyncPinState.HIGH)
    LOW_MESSAGE = SyncPinMessage(SyncPinState.LOW)
    ack = handle_message(HIGH_MESSAGE.to_bytes(), ot3_state)
    assert ack == HIGH_MESSAGE.to_bytes()
    assert ot3_state.get_sync_pin_state()

    ack = handle_message(LOW_MESSAGE.to_bytes(), ot3_state)
    assert ack == LOW_MESSAGE.to_bytes()
    assert not ot3_state.get_sync_pin_state()
