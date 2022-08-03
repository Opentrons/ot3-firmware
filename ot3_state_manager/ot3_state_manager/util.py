"""Functions and classes that are shared across the package."""

from __future__ import annotations

import enum
import hashlib


class Direction(enum.Enum):
    """Class representing direction to move axis in."""

    POSITIVE = "+"
    NEGATIVE = "-"

    @classmethod
    def from_symbol(cls, symbol: str) -> "Direction":
        """Parse plus or minus symbol into direction."""
        if symbol == "+":
            return cls.POSITIVE
        elif symbol == "-":
            return cls.NEGATIVE
        else:
            raise ValueError(
                f'Symbol "{symbol}" is not valid.' f' Valid symbols are "+" and "-"'
            )


def get_md5_hash(data: bytes) -> bytes:
    """Convert bytes into a md5 hash."""
    return hashlib.md5(data).hexdigest().encode()
