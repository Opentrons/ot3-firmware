from typing import Type

import pytest
from opentrons.hardware_control.types import OT3Axis

from ot3_state_manager.messages import (
    MoveMessage,
    SyncPinMessage,
    parse_message,
)
from ot3_state_manager.util import Direction


@pytest.mark.parametrize(
    "message,expected_state",
    [
        ["SYNC_PIN HIGH", "HIGH"],
        ["SYNC_PIN LOW", "LOW"],
    ]
)
def test_sync_pin_message_to_message(message: str, expected_state: str) -> None:
    assert SyncPinMessage.to_message(message).state == expected_state


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
        "SYNC_PIN TOGGLE"
    ]
)
def test_invalid_sync_pin_message_to_message(message: str) -> None:
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
    ]
)
def test_move_message_to_message(
    message: str, expected_axis: OT3Axis, expected_direction: Direction
) -> None:
    assert MoveMessage.to_message(message).axis == expected_axis
    assert MoveMessage.to_message(message).direction == expected_direction


@pytest.mark.parametrize(
    "message",
    [
        "X",
        "X +",
        "= X",
        "+ F",
        "F",
    ]
)
def test_invalid_move_message_to_message(message: str) -> None:
    assert MoveMessage.to_message(message) is None


@pytest.mark.parametrize(
    "message,expected_type",
    [
        ["+ X", MoveMessage],
        ["SYNC_PIN HIGH", SyncPinMessage],
    ]
)
def test_parse_message(
    message: str, expected_type: Type
) -> None:
    assert isinstance(parse_message(message), expected_type)


@pytest.mark.parametrize(
    "message",
    [
        "X",
        "X +",
        "= X",
        "+ F",
        "F",
    ]
)
def test_invalid_parse_message(message: str) -> None:
    with pytest.raises(ValueError) as err:
        parse_message(message)
        assert err.value == f'Not able to parse message: "{message}"'
