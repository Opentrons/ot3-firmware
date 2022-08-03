"""Test for OT3StateManager object."""

import asyncio
from typing import Generator

import pytest
from opentrons.hardware_control.types import OT3Axis

from ot3_state_manager.ot3_state import OT3State
from ot3_state_manager.ot3_state_manager import OT3StateManager
from ot3_state_manager.pipette_model import PipetteModel
from tests.udp_client import (
    EchoClientProtocol,
)

HOST = "localhost"
PORT = 8088


@pytest.fixture(scope="function")
def ot3_state() -> OT3State:
    """Create OT3State object."""
    return OT3State.build(
        PipetteModel.SINGLE_20.value, PipetteModel.SINGLE_20.value, True
    )

@pytest.fixture
def loop(event_loop: asyncio.AbstractEventLoop) -> asyncio.AbstractEventLoop:
    yield event_loop

@pytest.fixture
def server(loop: asyncio.AbstractEventLoop, ot3_state: OT3State) -> Generator:
    """Start OT3StateManager server."""
    cancel_handle = asyncio.ensure_future(
        OT3StateManager(ot3_state).start_server(HOST, PORT), loop=loop
    )
    loop.run_until_complete(asyncio.sleep(0.01))

    try:
        yield
    finally:
        cancel_handle.cancel()

@pytest.fixture
async def client(loop: asyncio.AbstractEventLoop) -> EchoClientProtocol:
    client = await EchoClientProtocol.build(HOST, PORT, loop)
    await client.is_connected()
    yield client
    await client.close()


@pytest.mark.parametrize(
    "message,expected_val",
    (
        pytest.param("+ X", 1, id="pos_pulse"),
        pytest.param("- X", -1, id="neg_pulse"),
    ),
)
async def test_message_received(
    message: str,
    expected_val: int,
    server: None,
    client: EchoClientProtocol,
    loop: asyncio.AbstractEventLoop,
    ot3_state: OT3State
) -> None:
    """Confirm that pulse messages work correctly."""
    await asyncio.ensure_future(client.send_message(message), loop=loop)
    print("Performing assert")
    assert ot3_state.axis_current_position(OT3Axis.X) == expected_val
