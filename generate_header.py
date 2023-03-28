"""Script to generate c++ header file of canbus constants."""
from __future__ import annotations

import argparse
import importlib
import io
import os.path
import sys
from types import ModuleType
from header_generation_utils.header_generation_utils.utils import (
    Block,
    Languge,
    Subsystem,
    generate_file_comment,
    write_enum_cpp,
    write_enum_c,
)


def generate_can_cpp(output: io.StringIO, constants_mod: ModuleType) -> None:
    """Generate source code into output."""
    with Block(
            output=output,
            start="namespace can::ids {\n\n",
            terminate="}  // namespace can::ids\n",
    ):
        write_enum_cpp(constants_mod.FunctionCode, output)
        write_enum_cpp(constants_mod.MessageId, output)
        write_enum_cpp(constants_mod.NodeId, output)
        write_enum_cpp(constants_mod.ErrorCode, output)
        write_enum_cpp(constants_mod.ErrorSeverity, output)
        write_enum_cpp(constants_mod.ToolType, output)
        write_enum_cpp(constants_mod.SensorType, output)
        write_enum_cpp(constants_mod.SensorId, output)
        write_enum_cpp(constants_mod.SensorOutputBinding, output)
        write_enum_cpp(constants_mod.SensorThresholdMode, output)
        write_enum_cpp(constants_mod.PipetteTipActionType, output)
        write_enum_cpp(constants_mod.GearMotorId, output)
        write_enum_cpp(constants_mod.MotorPositionFlags, output)
        write_enum_cpp(constants_mod.MoveStopCondition, output)
        write_enum_cpp(constants_mod.PipetteType, output)


def generate_can_c(output: io.StringIO, constants_mod: ModuleType, arbitration_id: ModuleType) -> None:
    """Generate source code into output."""
    can = "CAN"
    write_enum_c(constants_mod.FunctionCode, output, can)
    write_enum_c(constants_mod.MessageId, output, can)
    write_enum_c(constants_mod.NodeId, output, can)
    write_enum_c(constants_mod.ErrorCode, output, can)
    write_enum_c(constants_mod.ToolType, output, can)
    write_enum_c(constants_mod.SensorType, output, can)
    write_enum_c(constants_mod.SensorId, output, can)
    write_enum_c(constants_mod.SensorOutputBinding, output, can)
    write_enum_c(constants_mod.SensorThresholdMode, output, can)
    write_enum_c(constants_mod.PipetteTipActionType, output, can)
    write_enum_c(constants_mod.GearMotorId, output, can)
    write_enum_c(constants_mod.MotorPositionFlags, output, can)
    write_enum_c(constants_mod.MoveStopCondition, output, can)
    write_arbitration_id_c(output, can, arbitration_id)
    write_enum_c(constants_mod.PipetteType, output, can)

def generate_rearpanel_cpp(output: io.StringIO, constants_mod: ModuleType) -> None:
    """Generate source code into output."""
    with Block(
            output=output,
            start="#include <cstdint>\n\nnamespace rearpanel::ids {\n\n",
            terminate="}  // namespace rearpanel::ids\n",
    ):
        write_enum_cpp(constants_mod.BinaryMessageId, output, 'uint16_t')
        write_enum_cpp(constants_mod.LightTransitionType, output, 'uint8_t')
        write_enum_cpp(constants_mod.LightAnimationType, output, 'uint8_t')


def write_arbitration_id_c(output: io.StringIO, prefix: str, arbitration_id: ModuleType) -> None:
    """Generate C arbitration id code."""
    output.write(f"/** {arbitration_id.ArbitrationIdParts.__doc__} */\n")
    with Block(
            output=output,
            start="typedef struct {\n",
            terminate=f"}} {prefix}{arbitration_id.ArbitrationIdParts.__name__};\n\n",
    ):
        for i in arbitration_id.ArbitrationIdParts._fields_:
            output.write(f"    unsigned int {i[0]}: {i[2]};\n")


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

    parser.add_argument(
        "--subsystem",
        metavar="subsystem",
        type=Subsystem,
        default=Subsystem.CAN,
        choices=Subsystem,
        nargs="?",
        help=f"subsystem to generate constants for. can be one of {','.join(s.value for s in Subsystem)}",
    )

    args = parser.parse_args()
    if args.source_dir:
        sys.path.append(os.path.join(args.source_dir, os.path.pardir))
    constants = importlib.import_module('opentrons_hardware.firmware_bindings.constants')
    arbid = importlib.import_module('opentrons_hardware.firmware_bindings.arbitration_id')
    rearpanel_constants = importlib.import_module('opentrons_hardware.firmware_bindings.binary_constants')
    output = args.target
    generate_file_comment(output)

    if args.subsystem in {Subsystem.CAN}:
        if args.language in {Languge.CPP, Languge.CPP_alt}:
            generate_can_cpp(output, constants)
        if args.language in {Languge.C}:
            generate_can_c(output, constants, arbid)
    elif args.subsystem in {Subsystem.REARPANEL}:
        if args.language in {Languge.CPP, Languge.CPP_alt}:
            generate_rearpanel_cpp(output, rearpanel_constants)
        if args.language in {Languge.C}:
            raise RuntimeError(f"C header unsupported for subsystem={args.subsystem}")



if __name__ == "__main__":
    main()
