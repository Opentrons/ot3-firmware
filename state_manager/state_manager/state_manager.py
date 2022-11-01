"""Class that communicates with hardware simulators and updates OT3State."""

from __future__ import annotations

import argparse
import asyncio
import logging
from asyncio import BaseProtocol
from typing import List, Tuple, cast

from .messages import handle_message
from .ot3_state import OT3State
from .pipette_model import PipetteModel

log = logging.getLogger(__name__)


class OT3StateManager(BaseProtocol):
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
        self._connected = False
        self._addresses: List[Tuple[str, str]] = []

    def connection_made(self, transport) -> None:  # noqa: ANN001
        """Called when connection is made from client."""
        self._connected = True
        self.transport = transport

    def datagram_received(self, data: bytes, addr: Tuple[str, str]) -> None:
        """Called when a datagram is received from client."""
        response = handle_message(data, self._ot3_state)
        if self.transport is not None and response is not None:
            self.transport.sendto(response, addr)

    async def start_server(self, host: str, port: int) -> OT3StateManager:
        """Starts OT3StateManager UDP Datagram server"""
        loop = asyncio.get_running_loop()
        transport, protocol = await loop.create_datagram_endpoint(
            lambda: self,
            local_addr=(host, port),
        )
        return cast(OT3StateManager, protocol)  # Casting for mypy

    async def is_connected(self) -> bool:
        """Flag for when a client is connected."""
        return self._connected

    async def close(self) -> None:
        """Close existing socket connection."""
        if self.transport is not None:
            self.transport.close()
        self._connected = False


async def main() -> None:
    """CLI interface."""
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
        help="The model name of the pipette to attach to the left mount. (Default none)",
        dest=LEFT_PIPETTE_DEST,
    )
    parser.add_argument(
        "--right-pipette",
        action="store",
        default=None,
        choices=PIPETTE_NAMES,
        help="The model name of the pipette to attach to the right mount. (Default none)",
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
    while 1:
        await asyncio.sleep(1)


if __name__ == "__main__":
    asyncio.run(main())
