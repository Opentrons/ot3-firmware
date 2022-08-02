"""Class that communicates with hardware simulators and updates OT3State."""

from __future__ import annotations

import asyncio
import logging
from typing import Optional

from ot3_state_manager.ot3_state import OT3State
from ot3_state_manager.util import (
    MoveMessage,
    SyncPinMessage,
    get_md5_hash,
    parse_message,
)

log = logging.getLogger(__name__)


class OT3StateManager:
    """Async socket server that accepts pulse messages."""

    def __init__(self, host: str, port: int, ot3_state: OT3State) -> None:
        """Creates an OT3StateManager Object.

        Args:
            host: hostname to connect to
            port: port to connect to
            ot3_state: OT3State object to modify
        """
        self._host = host
        self._port = port
        self._ot3_state = ot3_state

    async def _handle_message(
        self, reader: asyncio.StreamReader, writer: asyncio.StreamWriter
    ) -> None:
        data = await reader.read(100)
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
            writer.write(f"ERROR: {error_response}".encode())
        else:
            writer.write(get_md5_hash(data))
        await writer.drain()
        writer.close()

    async def start_server(self) -> None:
        """Starts asyncio server"""
        server = await asyncio.start_server(
            self._handle_message, self._host, self._port
        )

        addr = server.sockets[0].getsockname()
        log.info(f"SERVER: Serving on {addr[0:2]}")

        async with server:
            await server.serve_forever()
