import asyncio
import socket

import pytest

from opentrons.hardware_control.types import OT3Axis
from ot3_state_manager.ot3_state_manager import OT3StateManager
from ot3_state_manager.ot3_state import OT3State
from ot3_state_manager.pipette_model import PipetteModel

from ot3_state_manager.util import (
    Direction,
    get_md5_hash,
)

HOST = "localhost"
PORT = 8088


@pytest.fixture(scope="function")
def ot3_state() -> OT3State:
    return OT3State.build(
        PipetteModel.SINGLE_20,
        PipetteModel.SINGLE_20,
        True
    )


@pytest.fixture
def server(event_loop, ot3_state: OT3State) -> None:
    cancel_handle = asyncio.ensure_future(
        OT3StateManager(HOST, PORT, ot3_state).start_message_handler()
        , loop=event_loop
    )
    event_loop.run_until_complete(asyncio.sleep(0.01))

    try:
        yield
    finally:
        cancel_handle.cancel()


async def send_message(message: str) -> str:
    reader, writer = await asyncio.open_connection(HOST, PORT)
    writer.write(message.encode())
    await writer.drain()

    data = await reader.read(1000)
    writer.close()
    await writer.wait_closed()

    return data.decode()


@pytest.mark.parametrize(
    "message,expected_val", (
        pytest.param("+ X", 1, id="pos_pulse"),
        pytest.param("- X", -1, id="neg_pulse"),
    )
)
async def test_pulse(
    message: str, expected_val: int, server: None, ot3_state: OT3State
) -> None:
    expected_ack = get_md5_hash(message.encode())
    ack = await send_message(message)
    assert ack == expected_ack.decode()
    assert ot3_state.axis_current_position(OT3Axis.X) == expected_val


@pytest.mark.parametrize(
    "message, error", (
        pytest.param(
            "A Bad Message",
            "ERROR: Bad Message Format. Expects \"<axis> <direction>\". You passed \"A Bad Message\"",  # noqa: E501
            id="bad_message_format"
        ),
        pytest.param(
            "+ F",
            "ERROR: Invalid Axis Passed. You passed \"F\"",
            id="invalid_axis"
        ),
        pytest.param(
            "= X",
            "ERROR: Symbol \"=\" is not valid. Valid symbols are \"+\" and \"-\"",
            id="invalid_direction"
        ),
    )
)
async def test_bad_message(
    message: str, error: str, server: None, ot3_state: OT3State
) -> None:
    response = await send_message(message)
    assert response == error
    assert all([value == 0 for value in ot3_state.current_position.values()])
    assert all([value == 0 for value in ot3_state.encoder_position.values()])
