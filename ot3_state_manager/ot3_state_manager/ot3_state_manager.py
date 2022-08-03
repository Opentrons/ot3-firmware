"""Class that communicates with hardware simulators and updates OT3State."""

from __future__ import annotations

import argparse
import asyncio
import logging
from typing import Optional

from opentrons.hardware_control.types import OT3Axis

from ot3_state_manager.ot3_state import OT3State
from ot3_state_manager.pipette_model import PipetteModel
from ot3_state_manager.util import (
    MoveMessage,
    SyncPinMessage,
    get_md5_hash,
    parse_message,
)

log = logging.getLogger(__name__)


class OT3StateManager:
    """Async socket server that accepts pulse messages."""

    def __init__(self, ot3_state: OT3State) -> None:
        """Creates an OT3StateManager Object.

        Args:
            host: hostname to connect to
            port: port to connect to
            ot3_state: OT3State object to modify
        """
        self._ot3_state = ot3_state
        self.transport = None

    def connection_made(self, transport):
        print("Server: Connection Made")
        self.transport = transport

    def datagram_received(self, data, addr):
        error_response: Optional[str] = None
        try:
            message = parse_message(data.decode())
        except ValueError as err:
            error_response = f"{err.args[0]}"
        except:  # noqa: E722
            error_response = "Unhandled Exception"
        else:
            if isinstance(message, MoveMessage):
                self._ot3_state.pulse(message.axis, message.direction)
            elif isinstance(message, SyncPinMessage):
                self._ot3_state.set_sync_pin(message.state)
            else:
                error_response = "Parsed to an unhandled message."

        if error_response is not None:
            response = f"ERROR: {error_response}".encode()
        else:
            response = get_md5_hash(data)
        self.transport.sendto(response, addr)
        self.transport = None

    async def start_server(self, host, port) -> None:
        loop = asyncio.get_running_loop()
        await loop.create_datagram_endpoint(lambda: self, local_addr=(host, port))


async def main():
    LEFT_PIPETTE_DEST = "left_pipette_model_name"
    RIGHT_PIPETTE_DEST = "right_pipette_model_name"
    GRIPPER_DEST = "use_gripper"
    PIPETTE_NAMES = [member.value for member in PipetteModel]
    parser = argparse.ArgumentParser(description="OT-3 State Manager CLI")

    parser.add_argument(
        "host", action="store", help="Host IP or name to connect to.", type=str
    )
    parser.add_argument(
        "port", action="store", help="The port number to connect to.", type=int
    )

    parser.add_argument(
        "--left-pipette",
        action="store",
        default=None,
        choices=PIPETTE_NAMES,
        help="The model name of the pipette to attach to the left mount. (Default none)",  # noqa: E501
        dest=LEFT_PIPETTE_DEST,
    )
    parser.add_argument(
        "--right-pipette",
        action="store",
        default=None,
        choices=PIPETTE_NAMES,
        help="The model name of the pipette to attach to the right mount. (Default none)",  # noqa: E501
        dest=RIGHT_PIPETTE_DEST,
    )
    parser.add_argument(
        "--attach-gripper",
        action="store_true",
        default=False,
        help="Whether or not to attach a gripper. (Default False)",
        dest=GRIPPER_DEST,
    )
    arg_dict = parser.parse_args().__dict__
    host = arg_dict.pop("host")
    port = arg_dict.pop("port")
    ot3_state = OT3State.build(**arg_dict)
    await OT3StateManager(ot3_state).start_server(host, port)


if __name__ == "__main__":
    asyncio.run(main())
