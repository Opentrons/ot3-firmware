"""UDP Client for testing."""

from __future__ import annotations

import asyncio
from asyncio import BaseProtocol
from typing import Optional, Tuple, cast


class EchoClientProtocol(BaseProtocol):
    """UDP Client for testing."""

    def __init__(self) -> None:
        """Create EchoClientProtocol object."""
        self.transport = None
        self.connected = False

    def connection_made(self, transport) -> None:  # noqa: ANN001
        """Called when connection is made."""
        self.transport = transport
        self.connected = True

    @staticmethod
    def datagram_received(data: bytes, addr: Tuple[str, str]) -> None:
        """Called when datagram is received."""
        pass

    def error_received(self, exc) -> None:  # noqa: ANN001
        """Called when error is received."""
        pass

    def connection_lost(self, exc) -> None:  # noqa: ANN001
        """Called when connection is lost."""
        self.connected = False

    async def is_connected(self) -> bool:
        """Called when connected to server."""
        return self.connected

    async def close(self) -> None:
        """Close connection to server."""
        if self.transport is not None:
            self.transport.close()
        self.connected = False

    def send_message(self, message: str) -> None:
        """Send message to server."""
        if self.transport is not None:
            self.transport.sendto(message.encode())

    @classmethod
    async def build(
        cls, host: str, port: int, loop: Optional[asyncio.AbstractEventLoop] = None
    ) -> EchoClientProtocol:
        """Build EchoClientProtocol object."""
        if loop is None:
            loop = asyncio.get_running_loop()

        transport, protocol = await loop.create_datagram_endpoint(
            lambda: EchoClientProtocol(), remote_addr=(host, port)
        )
        return cast(EchoClientProtocol, protocol)  # Casting for mypy
