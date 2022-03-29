"""Script to generate c++ header file of canbus constants."""
from __future__ import annotations
import argparse
import io
from enum import Enum
from typing import Type, Any
from types import ModuleType
import sys
import os.path
import importlib

class block:
    """C block generator."""

    def __init__(self, output: io.StringIO, start: str, terminate: str) -> None:
        """Construct a code block context manager.

        Args:
            output: the buffer in which to write
            start: the text that begins the block
            terminate: the text that ends the block
        """
        self._output = output
        self._start = start
        self._terminate = terminate

    def __enter__(self) -> block:
        """Enter the context manager."""
        self._output.write(self._start)
        return self

    def __exit__(self, *exc: Any) -> None:
        """Exit the context manager."""
        self._output.write(self._terminate)


def generate_file_comment(output: io.StringIO) -> None:
    """Generate the header file comment."""
    output.write("/********************************************\n")
    output.write(" * This is a generated file. Do not modify.  *\n")
    output.write(" ********************************************/\n")
    output.write("#pragma once\n\n")


def generate_cpp(output: io.StringIO, constants_mod: ModuleType) -> None:
    """Generate source code into output."""
    generate_file_comment(output)
    with block(
        output=output,
        start="namespace can_ids {\n\n",
        terminate="}  // namespace can_ids\n\n",
    ):
        write_enum_cpp(constants_mod.FunctionCode, output)
        write_enum_cpp(constants_mod.MessageId, output)
        write_enum_cpp(constants_mod.NodeId, output)
        write_enum_cpp(constants_mod.ErrorCode, output)
        write_enum_cpp(constants_mod.ToolType, output)
        write_enum_cpp(constants_mod.SensorType, output)
        write_enum_cpp(constants_mod.SensorOutputBinding, output)


def write_enum_cpp(e: Type[Enum], output: io.StringIO) -> None:
    """Generate enum class from enumeration."""
    output.write(f"/** {e.__doc__} */\n")
    with block(
        output=output, start=f"enum class {e.__name__} {{\n", terminate="};\n\n"
    ):
        for i in e:
            output.write(f"    {i.name} = 0x{i.value:x},\n")


def generate_c(output: io.StringIO, constants_mod: ModuleType, arbitration_id: ModuleType) -> None:
    """Generate source code into output."""
    generate_file_comment(output)
    can = "CAN"
    write_enum_c(constants_mod.FunctionCode, output, can)
    write_enum_c(constants_mod.MessageId, output, can)
    write_enum_c(constants_mod.NodeId, output, can)
    write_enum_c(constants_mod.ErrorCode, output, can)
    write_enum_c(constants_mod.ToolType, output, can)
    write_arbitration_id_c(output, can, arbitration_id)


def write_enum_c(e: Type[Enum], output: io.StringIO, prefix: str) -> None:
    """Generate constants from enumeration."""
    output.write(f"/** {e.__doc__} */\n")
    with block(
        output=output,
        start="typedef enum {\n",
        terminate=f"}} {prefix}{e.__name__};\n\n",
    ):
        for i in e:
            name = "_".join(("can", e.__name__, i.name)).lower()
            output.write(f"    {name} = 0x{i.value:x},\n")


def write_arbitration_id_c(output: io.StringIO, prefix: str, arbitration_id: ModuleType) -> None:
    """Generate C arbitration id code."""
    output.write(f"/** {arbitration_id.ArbitrationIdParts.__doc__} */\n")
    with block(
        output=output,
        start="typedef struct {\n",
        terminate=f"}} {prefix}{arbitration_id.ArbitrationIdParts.__name__};\n\n",
    ):
        for i in arbitration_id.ArbitrationIdParts._fields_:
            output.write(f"    unsigned int {i[0]}: {i[2]};\n")


class Languge(str, Enum):
    """Langauge enum."""

    C = "c"
    CPP = "c++"
    CPP_alt = "cpp"


def main() -> None:
    """Entry point."""
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
    parser.add_argument(
        "-s",
        "--source-dir",
        type=str,
        default="",
        help="a path containing an importable opentrons_hardware. if not specified, will use installed package",
    )


    parser.add_argument(
        "--language",
        metavar="language",
        type=Languge,
        default=Languge.CPP,
        choices=Languge,
        nargs="?",
        help=f"language to use. can be one of {','.join(l.value for l in Languge)}",
    )

    args = parser.parse_args()
    if args.source_dir:
        sys.path.append(os.path.join(args.source_dir, os.path.pardir))
    constants = importlib.import_module('opentrons_hardware.firmware_bindings.constants')
    arbid = importlib.import_module('opentrons_hardware.firmware_bindings.arbitration_id')
    if args.language in {Languge.CPP, Languge.CPP_alt}:
        generate_cpp(args.target, constants)
    if args.language in {Languge.C}:
        generate_c(args.target, constants, arbid)


if __name__ == "__main__":
    main()
