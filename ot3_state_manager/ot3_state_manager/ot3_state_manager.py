from __future__ import annotations
import asyncio
import hashlib
import logging
from ot3_state_manager.ot3_state import OT3State
from ot3_state_manager.util import (
    Message,
    get_md5_hash,
)

log = logging.getLogger(__name__)


class OT3StateManager:
    def __init__(self, host: str, port: int, ot3_state:OT3State) -> None:
        self._host = host
        self._port = port
        self._ot3_state = ot3_state

    async def _handle_message(self, reader: asyncio.StreamReader, writer: asyncio.StreamWriter):
        data = await reader.read(100)
        self._ot3_state.pulse(Message.to_message(data.decode()))
        writer.write(get_md5_hash(data))
        await writer.drain()
        writer.close()

    async def start_message_handler(self):
        server = await asyncio.start_server(self._handle_message, self._host, self._port)

        addr = server.sockets[0].getsockname()
        log.info(f'SERVER: Serving on {addr[0:2]}')

        async with server:
            await server.serve_forever()
