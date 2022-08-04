"""Test message functionality."""

from typing import Type

import pytest
from opentrons.hardware_control.types import OT3Axis

from ot3_state_manager.messages import (
    MoveMessage,
    SyncPinMessage,
    handle_message,
    parse_message,
)
from ot3_state_manager.ot3_state import OT3State
from ot3_state_manager.util import Direction, get_md5_hash


@pytest.mark.parametrize(
    "message,expected_state",
    [
        ["SYNC_PIN HIGH", "HIGH"],
        ["SYNC_PIN LOW", "LOW"],
    ],
)
def test_sync_pin_message_to_message(message: str, expected_state: str) -> None:
    """Confirm that sync pin messages parse correctly."""
    sync_pin_message = SyncPinMessage.to_message(message)
    assert sync_pin_message is not None
    assert sync_pin_message.state == expected_state


@pytest.mark.parametrize(
    "message",
    [
        "SYNC_PIN_HIGH",
        "SYNC PIN HIGH",
        "SYNC PIN_HIGH",
        "SYNCPINHIGH",
        "SYNCPIN_HIGH",
        "SYNC_PINHIGH",
        "BLARGH",
        "SYNC_PIN TOGGLE",
    ],
)
def test_invalid_sync_pin_message_to_message(message: str) -> None:
    """Confirm invalid sync pin messages do not parse."""
    assert SyncPinMessage.to_message(message) is None


@pytest.mark.parametrize(
    "message,expected_axis,expected_direction",
    [
        ["+ X", OT3Axis.X, Direction.POSITIVE],
        ["- X", OT3Axis.X, Direction.NEGATIVE],
        ["+ Y", OT3Axis.Y, Direction.POSITIVE],
        ["- Y", OT3Axis.Y, Direction.NEGATIVE],
        ["+ Z_L", OT3Axis.Z_L, Direction.POSITIVE],
        ["- Z_L", OT3Axis.Z_L, Direction.NEGATIVE],
        ["+ Z_R", OT3Axis.Z_R, Direction.POSITIVE],
        ["- Z_R", OT3Axis.Z_R, Direction.NEGATIVE],
        ["+ Z_G", OT3Axis.Z_G, Direction.POSITIVE],
        ["- Z_G", OT3Axis.Z_G, Direction.NEGATIVE],
        ["+ P_L", OT3Axis.P_L, Direction.POSITIVE],
        ["- P_L", OT3Axis.P_L, Direction.NEGATIVE],
        ["+ P_R", OT3Axis.P_R, Direction.POSITIVE],
        ["- P_R", OT3Axis.P_R, Direction.NEGATIVE],
        ["+ Q", OT3Axis.Q, Direction.POSITIVE],
        ["- Q", OT3Axis.Q, Direction.NEGATIVE],
        ["+ G", OT3Axis.G, Direction.POSITIVE],
        ["- G", OT3Axis.G, Direction.NEGATIVE],
    ],
)
def test_move_message_to_message(
    message: str, expected_axis: OT3Axis, expected_direction: Direction
) -> None:
    """Confirm that move messages are parsed correctly."""
    move_message = MoveMessage.to_message(message)
    assert move_message is not None
    assert move_message.axis == expected_axis
    assert move_message.direction == expected_direction


@pytest.mark.parametrize(
    "message",
    [
        "X",
        "X +",
        "= X",
        "+ F",
        "F",
    ],
)
def test_invalid_move_message_to_message(message: str) -> None:
    """Confirm that invalid move messages do not parse."""
    assert MoveMessage.to_message(message) is None


@pytest.mark.parametrize(
    "message,expected_type",
    [
        ["+ X", MoveMessage],
        ["SYNC_PIN HIGH", SyncPinMessage],
    ],
)
def test_parse_message(message: str, expected_type: Type) -> None:
    """Confirm that parse_message function works correctly."""
    assert isinstance(parse_message(message), expected_type)


@pytest.mark.parametrize(
    "message",
    [
        "X",
        "X +",
        "= X",
        "+ F",
        "F",
    ],
)
def test_invalid_parse_message(message: str) -> None:
    """Confirm parse_message throws ValueError when unable parse to a known message."""
    with pytest.raises(ValueError) as err:
        parse_message(message)
        assert err.value == f'Not able to parse message: "{message}"'


@pytest.mark.parametrize(
    "message,expected_val",
    (
        pytest.param("+ X", 1, id="pos_pulse"),
        pytest.param("- X", -1, id="neg_pulse"),
    ),
)
def test_valid_handle_move_message(
    message: str, expected_val: int, ot3_state: OT3State
) -> None:
    """Confirm that pulse messages work correctly."""
    ack = handle_message(message.encode(), ot3_state)
    assert ack == get_md5_hash(message.encode())
    assert ot3_state.axis_current_position(OT3Axis.X) == expected_val


def test_valid_handle_sync_pin_message(ot3_state: OT3State) -> None:
    """Confirm that pulse messages work correctly."""
    HIGH_MESSAGE = "SYNC_PIN HIGH"
    LOW_MESSAGE = "SYNC_PIN LOW"

    ack = handle_message(HIGH_MESSAGE.encode(), ot3_state)
    assert ack == get_md5_hash(HIGH_MESSAGE.encode())
    assert ot3_state.get_sync_pin_state()

    ack = handle_message(LOW_MESSAGE.encode(), ot3_state)
    assert ack == get_md5_hash(LOW_MESSAGE.encode())
    assert not ot3_state.get_sync_pin_state()


@pytest.mark.parametrize(
    "message, expected_error",
    (
        pytest.param(
            "A Bad Message",
            'ERROR: Not able to parse message: "A Bad Message"',
            id="bad_message_format",
        ),
        pytest.param(
            "+ F", 'ERROR: Not able to parse message: "+ F"', id="invalid_axis"
        ),
        pytest.param(
            "= X", 'ERROR: Not able to parse message: "= X"', id="invalid_direction"
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
