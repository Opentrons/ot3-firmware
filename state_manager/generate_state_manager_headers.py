"""Script to generate hpp file with OT3StateManager message enums."""

import argparse
import sys
from enum import Enum
from typing import Type

from header_generation_utils.utils import generate_file_comment, write_enum_cpp

from state_manager.messages import MessageID
from state_manager.util import Direction, MoveMessageHardware, SyncPinState

MESSAGE_ID_NAME = "message-ids"
MOVE_MESSAGE_HARDWARE_ID_NAME = "move-message-hardware-ids"
DIRECTION_NAME = "direction"
SYNC_PIN_STATE_NAME = "sync-pin-state"
POSSIBLE_CHOICES = [
    MESSAGE_ID_NAME,
    MOVE_MESSAGE_HARDWARE_ID_NAME,
    DIRECTION_NAME,
    SYNC_PIN_STATE_NAME,
]


def get_message_id_enum() -> Type[Enum]:
    """Creates a MessageID enum.

    Cannot just pass the MessageID type to write_enum_cpp because MessageID
    is a multi attribute enum. For each enum entry there is a message_id and
    a builder_func attribute. Need to return an enum with just the message_id
    as the value.
    """
    message_ids = [(message.name.lower(), message.message_id) for message in MessageID]
    # mypy does not execute code, so it cannot figure out what the types
    # of the values inside the tuple are.
    # see https://github.com/python/mypy/issues/5317
    message_id_enum = Enum("MessageID", message_ids)  # type: ignore[misc]
    message_id_enum.__doc__ = MessageID.__doc__
    return message_id_enum


def get_move_message_hw_id_enum() -> Type[Enum]:
    """Creates MoveMessage hw_id enum.

    Cannot just pass the MoveMessage type to write_enum_cpp because MoveMessage
    is a multi attribute enum. For each enum entry there is a hw_id and
    a builder_func attribute. Need to return an enum with just the hw_id
    as the value.
    """
    move_message_hw_ids = [
        (message.axis.name.lower(), message.hw_id) for message in MoveMessageHardware
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
        "header_to_generate",
        metavar="HEADER_TO_GENERATE",
        type=str,
        choices=POSSIBLE_CHOICES,
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

    if args.header_to_generate == MESSAGE_ID_NAME:
        write_enum_cpp(get_message_id_enum(), output)
    elif args.header_to_generate == MOVE_MESSAGE_HARDWARE_ID_NAME:
        write_enum_cpp(get_move_message_hw_id_enum(), output)
    elif args.header_to_generate == DIRECTION_NAME:
        write_enum_cpp(Direction, output)
    elif args.header_to_generate == SYNC_PIN_STATE_NAME:
        write_enum_cpp(SyncPinState, output)
    else:
        raise ValueError("Value passed to header_to_generate arg was not recognized")


if __name__ == "__main__":
    main()
