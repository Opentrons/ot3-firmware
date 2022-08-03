import pytest

from ot3_state_manager.ot3_state import OT3State
from ot3_state_manager.pipette_model import PipetteModel


@pytest.fixture
def ot3_state() -> OT3State:
    """Returns OT3State with left/right pipette and a gripper."""
    return OT3State.build(
        left_pipette_model_name=PipetteModel.SINGLE_20.value,
        right_pipette_model_name=PipetteModel.MULTI_8_20.value,
        use_gripper=True,
    )
