"""OT3StateManager object tests."""
import pytest

from ot3_state_manager.hardware import (
    Gripper,
    LeftPipette,
    RightPipette,
)
from ot3_state_manager.ot3_axis import OT3Axis
from ot3_state_manager.ot3_state_manager import OT3StateManager
from ot3_state_manager.pipette_model import PipetteModel


@pytest.fixture
def ot3_state_manager() -> OT3StateManager:
    """Returns OT3StateManager with left/right pipette and a gripper."""
    return OT3StateManager.build(
        left_pipette_model=PipetteModel.SINGLE_20,
        right_pipette_model=PipetteModel.MULTI_8_20,
        use_gripper=True,
    )


@pytest.fixture
def ot3_state_manager_no_pipettes_or_gripper() -> OT3StateManager:
    """Returns OT3StateManager without left/right pipette and a gripper."""
    return OT3StateManager.build(
        left_pipette_model=None, right_pipette_model=None, use_gripper=False
    )


def test_build(ot3_state_manager: OT3StateManager) -> None:
    """Confirms that when building OT3StateManager with pipettes and gripper that it is configured correctly."""
    assert ot3_state_manager.left_pipette is not None
    assert ot3_state_manager.right_pipette is not None
    assert ot3_state_manager.gripper is not None

    assert isinstance(ot3_state_manager.left_pipette, LeftPipette)
    assert isinstance(ot3_state_manager.right_pipette, RightPipette)
    assert isinstance(ot3_state_manager.gripper, Gripper)

    assert ot3_state_manager.is_left_pipette_attached
    assert ot3_state_manager.is_right_pipette_attached
    assert ot3_state_manager.is_gripper_attached


def test_build_no_gripper() -> None:
    """Confirms that when building OT3StateManager no gripper that it is configured correctly."""
    manager = OT3StateManager.build(
        left_pipette_model=PipetteModel.SINGLE_20,
        right_pipette_model=PipetteModel.MULTI_8_20,
        use_gripper=False,
    )

    assert manager.left_pipette is not None
    assert manager.right_pipette is not None
    assert manager.gripper is None

    assert isinstance(manager.left_pipette, LeftPipette)
    assert isinstance(manager.right_pipette, RightPipette)

    assert manager.is_left_pipette_attached
    assert manager.is_right_pipette_attached
    assert not manager.is_gripper_attached


def test_build_no_left_pipette() -> None:
    """Confirms that when building OT3StateManager with no left pipette that it is configured correctly."""
    manager = OT3StateManager.build(
        left_pipette_model=None,
        right_pipette_model=PipetteModel.MULTI_8_20,
        use_gripper=True,
    )

    assert manager.left_pipette is None
    assert manager.right_pipette is not None
    assert manager.gripper is not None

    assert isinstance(manager.right_pipette, RightPipette)
    assert isinstance(manager.gripper, Gripper)

    assert not manager.is_left_pipette_attached
    assert manager.is_right_pipette_attached
    assert manager.is_gripper_attached


def test_build_no_right_pipette() -> None:
    """Confirms that when building OT3StateManager with no right pipette that it is configured correctly."""
    manager = OT3StateManager.build(
        left_pipette_model=PipetteModel.SINGLE_20,
        right_pipette_model=None,
        use_gripper=True,
    )

    assert manager.left_pipette is not None
    assert manager.right_pipette is None
    assert manager.gripper is not None

    assert isinstance(manager.left_pipette, LeftPipette)
    assert isinstance(manager.gripper, Gripper)

    assert manager.is_left_pipette_attached
    assert not manager.is_right_pipette_attached
    assert manager.is_gripper_attached


def test_high_throughput_pipette_wrong_mount() -> None:
    """Confirms that exception is thrown when trying to mount high-throughput pipette to left pipette mount."""
    with pytest.raises(ValueError) as err:
        OT3StateManager.build(
            left_pipette_model=PipetteModel.MULTI_96_1000,
            right_pipette_model=None,
            use_gripper=True,
        )
    assert err.match("High-throughput pipette must be connected to right pipette mount")


def test_high_throughput_too_many_pipettes() -> None:
    """Confirms that exception is thrown when trying to a left pipette when a high-throughput pipette is being mounted to the right slot."""
    with pytest.raises(ValueError) as err:
        OT3StateManager.build(
            left_pipette_model=PipetteModel.SINGLE_20,
            right_pipette_model=PipetteModel.MULTI_96_1000,
            use_gripper=True,
        )
    assert err.match(
        "When using high-throughput pipette you cannot have a pipette in the left "
        "mount"
    )


def test_high_throughput() -> None:
    """Confirms that is_high_throughput_pipette_attached returns True when high-throughput pipette is mounted."""
    manager = OT3StateManager.build(
        left_pipette_model=None,
        right_pipette_model=PipetteModel.MULTI_96_1000,
        use_gripper=False,
    )

    assert manager.is_high_throughput_pipette_attached
    assert not manager.is_left_pipette_attached
    assert manager.is_right_pipette_attached
    assert not manager.is_gripper_attached


def test_current_position(ot3_state_manager: OT3StateManager) -> None:
    """Confirms that .current_position returns correctly."""
    current_position_dict = ot3_state_manager.current_position
    assert set(current_position_dict.keys()) == {
        OT3Axis.X,
        OT3Axis.Y,
        OT3Axis.Z_L,
        OT3Axis.Z_R,
        OT3Axis.Z_G,
        OT3Axis.P_L,
        OT3Axis.P_R,
        OT3Axis.G,
    }
    assert all([val == 0.0 for val in current_position_dict.values()])


def test_current_position_no_gripper_or_pipettes(
    ot3_state_manager_no_pipettes_or_gripper: OT3StateManager,
) -> None:
    """Confirms that .current_position returns correctly when no pipettes or gripper is attached."""
    current_position_dict = ot3_state_manager_no_pipettes_or_gripper.current_position
    assert set(current_position_dict.keys()) == {
        OT3Axis.X,
        OT3Axis.Y,
        OT3Axis.Z_L,
        OT3Axis.Z_R,
        OT3Axis.Z_G,
        OT3Axis.P_L,
        OT3Axis.P_R,
        OT3Axis.G,
    }
    assert all(
        [
            current_position_dict[key] is None
            for key in [OT3Axis.G, OT3Axis.P_L, OT3Axis.P_R]
        ]
    )


def test_update_current_position(ot3_state_manager: OT3StateManager) -> None:
    """Confirms that updating the current position works."""
    ot3_state_manager.update_position(
        axis_to_update=OT3Axis.X,
        current_position=5.0,
        commanded_position=None,
        encoder_position=None,
    )
    assert ot3_state_manager.current_position[OT3Axis.X] == 5.0


def test_update_commanded_position(ot3_state_manager: OT3StateManager) -> None:
    """Confirms that updating the commanded position works."""
    ot3_state_manager.update_position(
        axis_to_update=OT3Axis.X,
        current_position=None,
        commanded_position=6.0,
        encoder_position=None,
    )
    assert ot3_state_manager.commanded_position[OT3Axis.X] == 6.0


def test_update_encoder_position(ot3_state_manager: OT3StateManager) -> None:
    """Confirms that updating the encoder position works."""
    ot3_state_manager.update_position(
        axis_to_update=OT3Axis.X,
        current_position=None,
        commanded_position=None,
        encoder_position=7.0,
    )
    assert ot3_state_manager.encoder_position[OT3Axis.X] == 7.0


def test_update_multiple_positions(ot3_state_manager: OT3StateManager) -> None:
    """Confirms that updating the multiple positions at the same time works."""
    ot3_state_manager.update_position(
        axis_to_update=OT3Axis.X,
        current_position=8.0,
        commanded_position=9.0,
        encoder_position=10.0,
    )
    assert ot3_state_manager.current_position[OT3Axis.X] == 8.0
    assert ot3_state_manager.commanded_position[OT3Axis.X] == 9.0
    assert ot3_state_manager.encoder_position[OT3Axis.X] == 10.0


def test_update_position_hardware_not_attached(
    ot3_state_manager_no_pipettes_or_gripper: OT3StateManager,
) -> None:
    """Confirms that updating a position of a piece of equipment, that is not attached, throws an exception."""
    with pytest.raises(ValueError) as err:
        ot3_state_manager_no_pipettes_or_gripper.update_position(
            axis_to_update=OT3Axis.G,
            current_position=6.0,
            commanded_position=None,
            encoder_position=None,
        )

    assert err.match(
        'Axis "G" is not available. Cannot update it\'s position.'
    )


def test_update_position_no_position_provided(
    ot3_state_manager: OT3StateManager,
) -> None:
    """Confirms that an exception is thrown when calling .update_position and no position is provided."""
    with pytest.raises(ValueError) as err:
        ot3_state_manager.update_position(
            axis_to_update=OT3Axis.G,
            current_position=None,
            commanded_position=None,
            encoder_position=None,
        )
    assert err.match("You must provide a least one position to update.")
