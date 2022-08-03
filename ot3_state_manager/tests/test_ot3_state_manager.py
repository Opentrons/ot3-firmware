"""Test for OT3StateManager object."""

import asyncio
from typing import Generator

import pytest
from opentrons.hardware_control.types import OT3Axis

from ot3_state_manager.ot3_state import OT3State
from ot3_state_manager.ot3_state_manager import OT3StateManager
from ot3_state_manager.pipette_model import PipetteModel
from ot3_state_manager.util import get_md5_hash

HOST = "localhost"
PORT = 8088


@pytest.fixture(scope="function")
def ot3_state() -> OT3State:
    """Create OT3State object."""
    return OT3State.build(
        PipetteModel.SINGLE_20.value, PipetteModel.SINGLE_20.value, True
    )


@pytest.fixture
def server(event_loop: asyncio.AbstractEventLoop, ot3_state: OT3State) -> Generator:
    """Start OT3StateManager server."""
    cancel_handle = asyncio.ensure_future(
        OT3StateManager(HOST, PORT, ot3_state).start_server(), loop=event_loop
    )
    event_loop.run_until_complete(asyncio.sleep(0.01))

    try:
        yield
    finally:
        cancel_handle.cancel()


async def send_message(message: str) -> str:
    """Send message to OT3StateManager server."""
    reader, writer = await asyncio.open_connection(HOST, PORT)
    writer.write(message.encode())
    await writer.drain()

    data = await reader.read(1000)
    writer.close()
    await writer.wait_closed()

    return data.decode()


@pytest.mark.parametrize(
    "message,expected_val",
    (
        pytest.param("+ X", 1, id="pos_pulse"),
        pytest.param("- X", -1, id="neg_pulse"),
    ),
)
async def test_pulse(
    message: str, expected_val: int, server: None, ot3_state: OT3State
) -> None:
    """Confirm that pulse messages work correctly."""
    expected_ack = get_md5_hash(message.encode())
    ack = await send_message(message)
    assert ack == expected_ack.decode()
    assert ot3_state.axis_current_position(OT3Axis.X) == expected_val


@pytest.mark.parametrize(
    "message,expected_val",
    (
        pytest.param("SYNC_PIN HIGH", True, id="sync_pin_high"),
        pytest.param("SYNC_PIN LOW", False, id="sync_pin_low"),
    ),
)
async def test_sync_pin_state(
    message: str, expected_val: bool, server: None, ot3_state: OT3State
) -> None:
    """Confirm that SyncPinMessages work correctly"""
    expected_ack = get_md5_hash(message.encode())
    ack = await send_message(message)
    assert ack == expected_ack.decode()
    assert ot3_state.get_sync_pin_state() == expected_val


@pytest.mark.parametrize(
    "message, error",
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
async def test_bad_message(
    message: str, error: str, server: None, ot3_state: OT3State
) -> None:
    """Confirm that unsupported messages throw exceptions."""
    response = await send_message(message)
    assert response == error
    assert all([value == 0 for value in ot3_state.current_position.values()])
    assert all([value == 0 for value in ot3_state.encoder_position.values()])
