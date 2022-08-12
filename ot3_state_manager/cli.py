import argparse
import asyncio
from dataclasses import dataclass
from typing import (
    Any,
    Dict,
    Optional,
)

import uvicorn
from fastapi import FastAPI

from ot3_state_manager.api_server import get_app
from ot3_state_manager.ot3_state import OT3State
from ot3_state_manager.ot3_state_manager import OT3StateManager
from ot3_state_manager.pipette_model import PipetteModel

LEFT_PIPETTE_DEST = "left_pipette_model_name"
RIGHT_PIPETTE_DEST = "right_pipette_model_name"
GRIPPER_DEST = "use_gripper"
PIPETTE_NAMES = [member.value for member in PipetteModel]

app = FastAPI()

@app.get("/")
async def root():
    a = "a"
    b = "b" + a
    return {"hello world": b}

def cli() -> Dict[str, Any]:
    """CLI interface."""

    parser = argparse.ArgumentParser(description="OT-3 State Manager CLI")

    parser.add_argument(
        "host", action="store", help="Host IP or name host server on.", type=str
    )
    parser.add_argument(
        "socket_port",
        action="store",
        help="The socket_port number for socket connections.",
        type=int,
    )
    parser.add_argument(
        "api_port",
        action="store",
        help="The socket_port number for restful API connections.",
        type=int,
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
    return parser.parse_args().__dict__


async def run_socket_server(
    host: str,
    port: int,
    left_pipette: Optional[str],
    right_pipette: Optional[str],
    use_gripper: bool
) -> None:
    ot3_state = OT3State.build(
        left_pipette_model_name=left_pipette,
        right_pipette_model_name=right_pipette,
        use_gripper=use_gripper
    )
    server = await OT3StateManager(ot3_state).start_server(host, port)

    await server.is_connected()
    try:
        await asyncio.sleep(3600)  # Serve for 1 hour.
    finally:
        await server.close()


async def run_http_server(port: int) -> None:
    config = uvicorn.Config("cli:app", port=port, log_level="info")
    server = uvicorn.Server(config)
    await server.serve()


async def main():
    cli_args = cli()
    await asyncio.gather(
        run_http_server(cli_args["api_port"]),
        run_socket_server(
            cli_args["host"],
            cli_args["socket_port"],
            cli_args[LEFT_PIPETTE_DEST],
            cli_args[RIGHT_PIPETTE_DEST],
            cli_args[GRIPPER_DEST],
        )
    )

if __name__ == "__main__":

    asyncio.run(main())
