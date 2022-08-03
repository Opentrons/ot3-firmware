import asyncio


class EchoClientProtocol:
    def __init__(self, message, on_con_lost):
        self.message = message
        self.on_con_lost = on_con_lost
        self.transport = None

    def connection_made(self, transport):
        print("Client: Connection Made")
        self.transport = transport
        print('Sending message:', self.message)
        self.transport.sendto(self.message.encode())

    @staticmethod
    def datagram_received(data, addr):
        print("Received from server:", data.decode())

    def error_received(self, exc):
        print('Error received:', exc)

    def connection_lost(self, exc):
        print("Connection closed")
        self.on_con_lost.set_result(True)

async def send_message(host: str, port: int, message: str):
    loop = asyncio.get_running_loop()
    on_con_lost = loop.create_future()

    transport, protocol = await loop.create_datagram_endpoint(
        lambda: EchoClientProtocol(message, on_con_lost),
        remote_addr=(host, port)
    )
