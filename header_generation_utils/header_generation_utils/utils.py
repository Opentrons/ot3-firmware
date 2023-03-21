from __future__ import annotations

from enum import Enum
from io import StringIO
from typing import (
    Any,
    Type,
)


class Block:
    """C block generator."""

    def __init__(self, output: StringIO, start: str, terminate: str) -> None:
        """Construct a code block context manager.

        Args:
            output: the buffer in which to write
            start: the text that begins the block
            terminate: the text that ends the block
        """
        self._output = output
        self._start = start
        self._terminate = terminate

    def __enter__(self) -> Block:
        """Enter the context manager."""
        self._output.write(self._start)
        return self

    def __exit__(self, *exc: Any) -> None:
        """Exit the context manager."""
        self._output.write(self._terminate)


def generate_file_comment(output: StringIO) -> None:
    """Generate the header file comment."""
    output.write("/********************************************\n")
    output.write(" * This is a generated file. Do not modify.  *\n")
    output.write(" ********************************************/\n")
    output.write("#pragma once\n\n")


def write_enum_cpp(e: Type[Enum], output: StringIO) -> None:
    """Generate enum class from enumeration."""
    output.write(f"/** {e.__doc__} */\n")
    with Block(
            output=output, start=f"enum class {e.__name__} {{\n", terminate="};\n\n"
    ):
        for i in e:
            output.write(f"    {i.name} = 0x{i.value:x},\n")


def write_enum_c(e: Type[Enum], output: StringIO, prefix: str) -> None:
    """Generate constants from enumeration."""
    output.write(f"/** {e.__doc__} */\n")
    with Block(
            output=output,
            start="typedef enum {\n",
            terminate=f"}} {prefix}{e.__name__};\n\n",
    ):
        for i in e:
            name = "_".join(("can", e.__name__, i.name)).lower()
            output.write(f"    {name} = 0x{i.value:x},\n")


class Languge(str, Enum):
    """Langauge enum."""

    C = "c"
    CPP = "c++"
    CPP_alt = "cpp"


class Subsystem(str, Enum):
    """Subsystem enum."""

    CAN = "can"
    REARPANEL = "rearpanel"
