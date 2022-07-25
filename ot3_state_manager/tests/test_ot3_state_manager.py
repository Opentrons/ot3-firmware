"""OT3StateManager object tests."""
import pytest

from ot3_state_manager.hardware import (
    Gripper,
    LeftPipette,
    RightPipette,
    UpdatablePositions,
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
        "Z_G",
        "Z_L",
        "Z_R",
        "X",
        "Y",
        "P_L",
        "P_R",
        "G",
    }
    assert all([val == 0.0 for val in current_position_dict.values()])


def test_current_position_no_gripper_or_pipettes(
    ot3_state_manager_no_pipettes_or_gripper: OT3StateManager,
) -> None:
    """Confirms that .current_position returns correctly when no pipettes or gripper is attached."""
    current_position_dict = ot3_state_manager_no_pipettes_or_gripper.current_position
    assert set(current_position_dict.keys()) == {
        "Z_G",
        "Z_L",
        "Z_R",
        "X",
        "Y",
        "P_L",
        "P_R",
        "G",
    }
    assert all(
        [
            current_position_dict[key] is None
            for key in [OT3Axis.G.name, OT3Axis.P_L.name, OT3Axis.P_R.name]
        ]
    )


def test_update_current_position(ot3_state_manager: OT3StateManager) -> None:
    """Confirms that updating the current position works."""
    ot3_state_manager.update_position(
        pos_to_update=UpdatablePositions.GANTRY_X,
        current_position=5.0,
        commanded_position=None,
        encoder_position=None,
    )
    axis_key = UpdatablePositions.pos_to_axis(UpdatablePositions.GANTRY_X)
    assert ot3_state_manager.current_position[axis_key] == 5.0


def test_update_commanded_position(ot3_state_manager: OT3StateManager) -> None:
    """Confirms that updating the commanded position works."""
    ot3_state_manager.update_position(
        pos_to_update=UpdatablePositions.GANTRY_X,
        current_position=None,
        commanded_position=6.0,
        encoder_position=None,
    )
    axis_key = UpdatablePositions.pos_to_axis(UpdatablePositions.GANTRY_X)
    assert ot3_state_manager.commanded_position[axis_key] == 6.0


def test_update_encoder_position(ot3_state_manager: OT3StateManager) -> None:
    """Confirms that updating the encoder position works."""
    ot3_state_manager.update_position(
        pos_to_update=UpdatablePositions.GANTRY_X,
        current_position=None,
        commanded_position=None,
        encoder_position=7.0,
    )
    axis_key = UpdatablePositions.pos_to_axis(UpdatablePositions.GANTRY_X)
    assert ot3_state_manager.encoder_position[axis_key] == 7.0


def test_update_multiple_positions(ot3_state_manager: OT3StateManager) -> None:
    """Confirms that updating the multiple positions at the same time works."""
    ot3_state_manager.update_position(
        pos_to_update=UpdatablePositions.GANTRY_X,
        current_position=8.0,
        commanded_position=9.0,
        encoder_position=10.0,
    )
    axis_key = UpdatablePositions.pos_to_axis(UpdatablePositions.GANTRY_X)
    assert ot3_state_manager.current_position[axis_key] == 8.0
    assert ot3_state_manager.commanded_position[axis_key] == 9.0
    assert ot3_state_manager.encoder_position[axis_key] == 10.0


def test_update_position_hardware_not_attached(
    ot3_state_manager_no_pipettes_or_gripper: OT3StateManager,
) -> None:
    """Confirms that updating a position of a piece of equipment, that is not attached, throws an exception."""
    with pytest.raises(ValueError) as err:
        ot3_state_manager_no_pipettes_or_gripper.update_position(
            pos_to_update=UpdatablePositions.GRIPPER_EXTEND,
            current_position=6.0,
            commanded_position=None,
            encoder_position=None,
        )

    assert err.match(
        'Hardware "Gripper Extend" is not attached. Cannot update it\'s position.'
    )


def test_update_position_no_position_provided(
    ot3_state_manager: OT3StateManager,
) -> None:
    """Confirms that an exception is thrown when calling .update_position and no position is provided."""
    with pytest.raises(ValueError) as err:
        ot3_state_manager.update_position(
            pos_to_update=UpdatablePositions.GRIPPER_EXTEND,
            current_position=None,
            commanded_position=None,
            encoder_position=None,
        )
    assert err.match("You must provide a least one position to update.")


def test_updatable_positions(ot3_state_manager: OT3StateManager) -> None:
    """Confirms UpdatablePositions contains the correct values."""
    assert set(ot3_state_manager.updatable_positions) == {
        UpdatablePositions.GANTRY_X,
        UpdatablePositions.GANTRY_Y,
        UpdatablePositions.GRIPPER_EXTEND,
        UpdatablePositions.GRIPPER_MOUNT,
        UpdatablePositions.LEFT_PIPETTE_MOUNT,
        UpdatablePositions.RIGHT_PIPETTE_MOUNT,
        UpdatablePositions.LEFT_PIPETTE_PLUNGER,
        UpdatablePositions.RIGHT_PIPETTE_PLUNGER,
    }


def test_updatable_positions_no_gripper_or_pipettes(
    ot3_state_manager_no_pipettes_or_gripper: OT3StateManager,
) -> None:
    """Confirms UpdatablePositions omits pipettes and gripper when they are not attached."""
    assert set(ot3_state_manager_no_pipettes_or_gripper.updatable_positions) == {
        UpdatablePositions.GANTRY_X,
        UpdatablePositions.GANTRY_Y,
        UpdatablePositions.GRIPPER_MOUNT,
        UpdatablePositions.LEFT_PIPETTE_MOUNT,
        UpdatablePositions.RIGHT_PIPETTE_MOUNT,
    }
