"""Payloads tests."""
import pytest

from opentrons_ot3_firmware.messages import payloads
from opentrons_ot3_firmware import utils


@pytest.mark.parametrize(
    argnames=["address", "data", "expected_checksum"],
    argvalues=[
        [0xF0F0F0F0, bytes(range(56)), 0xF604],
        [0x0, b"", 0x0],
        [0x0, b"\x00", 0xFFFF],
        [0x12345678, b"\x05\x06\x07", 0xFED7],
    ],
)
def test_create_firmware_updata_data(
    address: int, data: bytes, expected_checksum: int
) -> None:
    """It should create a complete firmware update data payload."""
    obj = payloads.FirmwareUpdateData.create(address, data)
    assert obj == payloads.FirmwareUpdateData(
        address=utils.UInt32Field(address),
        num_bytes=utils.UInt8Field(len(data)),
        reserved=utils.UInt8Field(0),
        data=payloads.FirmwareUpdateDataField(data),
        checksum=utils.UInt16Field(expected_checksum),
    )
