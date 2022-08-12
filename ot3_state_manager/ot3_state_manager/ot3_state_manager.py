"""Class that communicates with hardware simulators and updates OT3State."""

from __future__ import annotations

import argparse
import asyncio
import logging
from asyncio import BaseProtocol
from typing import Tuple, cast

from ot3_state_manager.messages import handle_message
from ot3_state_manager.ot3_state import OT3State
from ot3_state_manager.pipette_model import PipetteModel

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

    def connection_made(self, transport) -> None:  # noqa: ANN001
        """Called when connection is made from client."""
        self._connected = True
        self.transport = transport

    def datagram_received(self, data: bytes, addr: Tuple[str, str]) -> None:
        """Called when a datagram is received from client."""
        if self.transport is not None:
            self.transport.sendto(handle_message(data, self._ot3_state), addr)

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
