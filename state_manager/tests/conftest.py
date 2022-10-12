"""Global fixtures available to all test files at the same level or below this file."""

import pytest

from state_manager.ot3_state import OT3State
from state_manager.pipette_model import PipetteModel


@pytest.fixture
def ot3_state() -> OT3State:
    """Returns OT3State with left/right pipette and a gripper."""
    return OT3State.build(
        left_pipette_model_name=PipetteModel.SINGLE_20.get_pipette_name(),
        right_pipette_model_name=PipetteModel.MULTI_8_20.get_pipette_name(),
        use_gripper=True,
    )
