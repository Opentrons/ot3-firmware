"""Test for OT3StateManager object."""

import asyncio
from typing import AsyncGenerator, Generator
from unittest.mock import Mock, patch

import pytest
from opentrons.hardware_control.types import Axis

from state_manager.messages import MoveMessage
from state_manager.ot3_state import OT3State
from state_manager.pipette_model import PipetteModel
from state_manager.state_manager import OT3StateManager
from state_manager.util import Direction
from tests.udp_client import EchoClientProtocol

HOST = "localhost"
PORT = 8088


@pytest.fixture(scope="function")
def ot3_state() -> OT3State:
    """Create OT3State object."""
    return OT3State.build(
        PipetteModel.SINGLE_20.value, PipetteModel.SINGLE_20.value, True
    )


@pytest.fixture
def loop(
    event_loop: asyncio.AbstractEventLoop,
) -> Generator[asyncio.AbstractEventLoop, None, None]:
    """Create and yield event_loop."""
    yield event_loop


@pytest.fixture
async def server(
    loop: asyncio.AbstractEventLoop, ot3_state: OT3State
) -> AsyncGenerator[OT3StateManager, None]:
    """Start OT3StateManager server."""
    server = await OT3StateManager(ot3_state).start_server(HOST, PORT)
    await server.is_connected()
    yield server
    await server.close()


@pytest.fixture
async def client(
    loop: asyncio.AbstractEventLoop,
) -> AsyncGenerator[EchoClientProtocol, None]:
    """Create UDP Client."""
    client = await EchoClientProtocol.build(HOST, PORT, loop)
    await client.is_connected()
    yield client
    await client.close()


@patch.object(OT3StateManager, "datagram_received")
async def test_message_received(
    patched_object: Mock,
    server: OT3StateManager,
    client: EchoClientProtocol,
    loop: asyncio.AbstractEventLoop,
    ot3_state: OT3State,
) -> None:
    """Confirm that pulse messages work correctly."""
    move_message_bytes = MoveMessage(Axis.X, Direction.POSITIVE).to_bytes()
    client.send_message(move_message_bytes)
    await asyncio.wait_for(server.is_connected(), 15.0)
    datagram_received_data_arg_content = patched_object.call_args.args[0]
    assert datagram_received_data_arg_content == move_message_bytes
