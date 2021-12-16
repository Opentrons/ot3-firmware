"""BinarySerializable dataclass."""

from __future__ import annotations
import struct
from dataclasses import dataclass, fields, astuple
from typing import TypeVar, Generic


class BinarySerializableException(BaseException):
    """Exception."""

    pass


class InvalidFieldException(BinarySerializableException):
    """Field is wrong type."""

    pass


class SerializationException(BinarySerializableException):
    """Serialization error."""

    pass


T = TypeVar("T")


class BinaryFieldBase(Generic[T]):
    """Binary serializable field."""

    FORMAT = ""
    """The struct format string for this field."""

    def __init__(self, t: T) -> None:
        """Constructor."""
        self._t = t

    @classmethod
    def build(cls, t: T) -> BinaryFieldBase[T]:
        """Factory method.

        Args:
            t: The value.

        Returns:
            New instance.
        """
        return cls(t)

    @property
    def value(self) -> T:
        """The value."""
        return self._t

    def __eq__(self, other: object) -> bool:
        """Comparison."""
        return isinstance(other, BinaryFieldBase) and other.value == self.value

    def __repr__(self) -> str:
        """Representation string."""
        return f"{self.__class__.__name__}(value={repr(self.value)})"


class UInt64Field(BinaryFieldBase[int]):
    """Unsigned 64 bit integer field."""

    FORMAT = "Q"


class Int64Field(BinaryFieldBase[int]):
    """Signed 64 bit integer field."""

    FORMAT = "q"


class UInt32Field(BinaryFieldBase[int]):
    """Unsigned 32 bit integer field."""

    FORMAT = "L"


class Int32Field(BinaryFieldBase[int]):
    """Signed 32 bit integer field."""

    FORMAT = "l"


class UInt16Field(BinaryFieldBase[int]):
    """Unsigned 16 bit integer field."""

    FORMAT = "H"


class Int16Field(BinaryFieldBase[int]):
    """Signed 16 bit integer field."""

    FORMAT = "h"


class UInt8Field(BinaryFieldBase[int]):
    """Unsigned 8 bit integer field."""

    FORMAT = "B"


class Int8Field(BinaryFieldBase[int]):
    """Signed 8 bit integer field."""

    FORMAT = "b"


@dataclass
class BinarySerializable:
    """Base class of a dataclass that can be serialized/deserialized into bytes.

    Each field must have the FieldMetaData metadata value defining the format string
    used by `struct` package to pack/unpack the field.

    Data will be packed big endian.
    """

    ENDIAN = ">"
    """The big endian format string"""

    def serialize(self) -> bytes:
        """Serialize into a byte buffer.

        Returns:
            Byte buffer
        """
        string = self._get_format_string()
        vals = [x.value for x in astuple(self)]
        try:
            return struct.pack(string, *vals)
        except struct.error as e:
            raise SerializationException(str(e))

    @classmethod
    def build(cls, data: bytes) -> BinarySerializable:
        """Create a BinarySerializable from a byte buffer.

        The byte buffer must be at least enough bytes to satisfy all fields.

        Extra bytes will be ignored. This is for two reasons:
            - CANFD requires padding to round byte lengths to fixed sizes.
            - To accommodate extracting multiple  BinarySerializable objects
            from a stream of bytes.

        Args:
            data: Byte buffer

        Returns:
            cls
        """
        try:
            format_string = cls._get_format_string()
            size = cls.get_size()
            # ignore bytes beyond the size of message.
            b = struct.unpack(format_string, data[:size])
            args = {v.name: v.type.build(b[i]) for i, v in enumerate(fields(cls))}
            # Mypy is not liking constructing the derived types.
            return cls(**args)  # type: ignore[call-arg]
        except struct.error as e:
            raise InvalidFieldException(str(e))

    @classmethod
    def _get_format_string(cls) -> str:
        """Get the `struct` format string for this class.

        Returns:
            a string
        """
        dataclass_fields = fields(cls)
        try:
            format_string = (
                f"{cls.ENDIAN}{''.join(v.type.FORMAT for v in dataclass_fields)}"
            )
        except AttributeError:
            raise InvalidFieldException(f"All fields must be of type {BinaryFieldBase}")

        return format_string

    @classmethod
    def get_size(cls) -> int:
        """Get the size of the serializable in bytes."""
        return struct.calcsize(cls._get_format_string())


class LittleEndianMixIn:
    """Mix in to use with BinarySerializable to use little endian packing."""

    ENDIAN = "<"
    """The little endian format string"""


class LittleEndianBinarySerializable(LittleEndianMixIn, BinarySerializable):
    """Little endian binary serializable."""

    ...
