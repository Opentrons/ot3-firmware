"""Test for constants module."""
from typing import Iterable

import pytest

from opentrons_ot3_firmware import constants


@pytest.mark.parametrize(argnames=["subject", "bit_width"],
                         argvalues=[
                             [constants.NodeId, constants.NODE_ID_BITS],
                             [constants.MessageId, constants.MESSAGE_ID_BITS],
                             [constants.FunctionCode, constants.FUNCTION_CODE_BITS]
                         ])
def test_range(subject: Iterable[int], bit_width: int) -> None:
    """Each value must be in range."""
    mask = (2 ** bit_width) - 1
    for v in subject:
        assert (v & ~mask) == 0, f"{v} must be between 0 and {mask}"
