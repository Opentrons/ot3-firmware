"""Script to generate hpp file with OT3StateManager message enums."""

import argparse
import os
import sys
from enum import Enum
from typing import Type

from ot3_state_manager.messages import MessageID
from ot3_state_manager.util import Direction, MoveMessageHardware, SyncPinState

# Have to do this weirdness because if this file gets moved to outside ot3_state_manager
# then the opentrons module cannot be imported.
module_path = os.path.normpath(
    os.path.join(os.path.dirname(os.path.abspath(__file__)), "..")
)
sys.path.append(module_path)
# Need to ignore E402 which is complaining about this import not being at the top
# of the file. It can't be because the weird import above needs to be performed.
from header_generation_utils import generate_file_comment, write_enum_cpp  # noqa: E402


def get_message_id_enum() -> Type[Enum]:
    """Creates a MessageID enum."""
    message_ids = [
        ("_".join([message.name, "message_id"]).lower(), message.message_id)
        for message in MessageID
    ]
    # mypy does not execute code, so it cannot figure out what the types
    # of the values inside the tuple are.
    # see https://github.com/python/mypy/issues/5317
    message_id_enum = Enum("MessageID", message_ids)  # type: ignore[misc]
    message_id_enum.__doc__ = MessageID.__doc__
    return message_id_enum


def get_move_message_hw_id_enum() -> Type[Enum]:
    """Creates MoveMessage hw_id enum."""
    move_message_hw_ids = [
        ("_".join([message.axis.name, "axis_hw_id"]).lower(), message.hw_id)
        for message in MoveMessageHardware
    ]
    # mypy does not execute code, so it cannot figure out what the types
    # of the values inside the tuple are.
    # see https://github.com/python/mypy/issues/5317
    move_message_hw_id_enum = Enum("MoveMessageHardware", move_message_hw_ids)  # type: ignore[misc]
    move_message_hw_id_enum.__doc__ = MoveMessageHardware.__doc__
    return move_message_hw_id_enum


def main() -> None:
    """Runs cli and generates header file."""
    parser = argparse.ArgumentParser(
        description="Generate a C or C++ header files defining CANBUS constants."
    )
    parser.add_argument(
        "target",
        metavar="TARGET",
        type=argparse.FileType("w"),
        default=sys.stdout,
        nargs="?",
        help="path of header file to generate; use - or do not specify for stdout",
    )

    args = parser.parse_args()
    output = args.target
    generate_file_comment(output)
    write_enum_cpp(get_message_id_enum(), output)
    write_enum_cpp(get_move_message_hw_id_enum(), output)
    write_enum_cpp(Direction, output)
    write_enum_cpp(SyncPinState, output)


if __name__ == "__main__":
    main()
