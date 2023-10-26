"""OT3State object tests."""
import pytest
from opentrons.hardware_control.types import Axis

from state_manager.hardware import Gripper, LeftPipette, RightPipette
from state_manager.ot3_state import OT3State
from state_manager.pipette_model import PipetteModel
from state_manager.util import Direction, SyncPinState


@pytest.fixture
def ot3_state_no_pipettes_or_gripper() -> OT3State:
    """Returns OT3State without left/right pipette and a gripper."""
    return OT3State.build(
        left_pipette_model_name=None, right_pipette_model_name=None, use_gripper=False
    )


def test_build(ot3_state: OT3State) -> None:
    """Confirms that when building OT3State with pipettes and gripper that it is configured correctly."""
    assert ot3_state.left_pipette is not None
    assert ot3_state.right_pipette is not None
    assert ot3_state.gripper is not None

    assert isinstance(ot3_state.left_pipette, LeftPipette)
    assert isinstance(ot3_state.right_pipette, RightPipette)
    assert isinstance(ot3_state.gripper, Gripper)

    assert ot3_state.is_left_pipette_attached
    assert ot3_state.is_right_pipette_attached
    assert ot3_state.is_gripper_attached


def test_build_no_gripper() -> None:
    """Confirms that when building OT3State no gripper that it is configured correctly."""
    manager = OT3State.build(
        left_pipette_model_name=PipetteModel.SINGLE_20.get_pipette_name(),
        right_pipette_model_name=PipetteModel.MULTI_8_20.get_pipette_name(),
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
    """Confirms that when building OT3State with no left pipette that it is configured correctly."""
    manager = OT3State.build(
        left_pipette_model_name=None,
        right_pipette_model_name=PipetteModel.MULTI_8_20.get_pipette_name(),
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
    """Confirms that when building OT3State with no right pipette that it is configured correctly."""
    manager = OT3State.build(
        left_pipette_model_name=PipetteModel.SINGLE_20.get_pipette_name(),
        right_pipette_model_name=None,
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
        OT3State.build(
            left_pipette_model_name=PipetteModel.MULTI_96_1000.get_pipette_name(),
            right_pipette_model_name=None,
            use_gripper=True,
        )
    assert err.match("High-throughput pipette must be connected to right pipette mount")


def test_high_throughput_too_many_pipettes() -> None:
    """Confirms that exception is thrown when trying to a left pipette when a high-throughput pipette is being mounted to the right slot."""
    with pytest.raises(ValueError) as err:
        OT3State.build(
            left_pipette_model_name=PipetteModel.SINGLE_20.get_pipette_name(),
            right_pipette_model_name=PipetteModel.MULTI_96_1000.get_pipette_name(),
            use_gripper=True,
        )
    assert err.match(
        "When using high-throughput pipette you cannot have a pipette in the left "
        "mount"
    )


def test_high_throughput() -> None:
    """Confirms that is_high_throughput_pipette_attached returns True when high-throughput pipette is mounted."""
    manager = OT3State.build(
        left_pipette_model_name=None,
        right_pipette_model_name=PipetteModel.MULTI_96_1000.get_pipette_name(),
        use_gripper=False,
    )

    assert manager.is_high_throughput_pipette_attached
    assert not manager.is_left_pipette_attached
    assert manager.is_right_pipette_attached
    assert not manager.is_gripper_attached


def test_current_position(ot3_state: OT3State) -> None:
    """Confirms that .current_position returns correctly."""
    current_position_dict = ot3_state.current_position
    assert set(current_position_dict.keys()) == {
        Axis.X,
        Axis.Y,
        Axis.Z_L,
        Axis.Z_R,
        Axis.Z_G,
        Axis.P_L,
        Axis.P_R,
        Axis.G,
    }
    assert all([val == 0 for val in current_position_dict.values()])


def test_current_position_no_gripper_or_pipettes(
    ot3_state_no_pipettes_or_gripper: OT3State,
) -> None:
    """Confirms that .current_position returns correctly when no pipettes or gripper is attached."""
    current_position_dict = ot3_state_no_pipettes_or_gripper.current_position
    assert set(current_position_dict.keys()) == {
        Axis.X,
        Axis.Y,
        Axis.Z_L,
        Axis.Z_R,
        Axis.Z_G,
        Axis.P_L,
        Axis.P_R,
        Axis.G,
    }
    assert all(
        [current_position_dict[key] is None for key in [Axis.G, Axis.P_L, Axis.P_R]]
    )


def test_update_current_position(ot3_state: OT3State) -> None:
    """Confirms that updating the current position works."""
    ot3_state.update_position(
        axis_to_update=Axis.X,
        current_position=5,
        encoder_position=0,
    )
    assert ot3_state.current_position[Axis.X] == 5


def test_update_encoder_position(ot3_state: OT3State) -> None:
    """Confirms that updating the encoder position works."""
    ot3_state.update_position(
        axis_to_update=Axis.X,
        current_position=0,
        encoder_position=7,
    )
    assert ot3_state.encoder_position[Axis.X] == 7


def test_update_multiple_positions(ot3_state: OT3State) -> None:
    """Confirms that updating the multiple positions at the same time works."""
    ot3_state.update_position(
        axis_to_update=Axis.X,
        current_position=8,
        encoder_position=10,
    )
    assert ot3_state.current_position[Axis.X] == 8
    assert ot3_state.encoder_position[Axis.X] == 10


def test_update_position_hardware_not_attached(
    ot3_state_no_pipettes_or_gripper: OT3State,
) -> None:
    """Confirms that updating a position of a piece of equipment, that is not attached, throws an exception."""
    with pytest.raises(ValueError) as err:
        ot3_state_no_pipettes_or_gripper.update_position(
            axis_to_update=Axis.G,
            current_position=6,
            encoder_position=0,
        )

    assert err.match('Axis "G" is not available. Cannot update it\'s position.')


def test_pulse() -> None:
    """Confirms that pulsing works correctly."""
    state_1 = OT3State.build(
        PipetteModel.SINGLE_20.get_pipette_name(),
        PipetteModel.SINGLE_20.get_pipette_name(),
        True,
    )
    state_2 = OT3State.build(
        PipetteModel.SINGLE_20.get_pipette_name(),
        PipetteModel.SINGLE_20.get_pipette_name(),
        True,
    )

    state_1.pulse(axis=Axis.X, direction=Direction.POSITIVE)
    assert state_1.axis_current_position(Axis.X) == 1
    assert state_2.axis_current_position(Axis.X) == 0

    state_1.pulse(axis=Axis.X, direction=Direction.POSITIVE)
    assert state_1.axis_current_position(Axis.X) == 2
    assert state_2.axis_current_position(Axis.X) == 0

    state_1.pulse(axis=Axis.X, direction=Direction.NEGATIVE)
    assert state_1.axis_current_position(Axis.X) == 1
    assert state_2.axis_current_position(Axis.X) == 0


def test_sync_pin(ot3_state: OT3State) -> None:
    """Confirm that sync pin methods function correctly."""
    assert not ot3_state.get_sync_pin_state()
    ot3_state.set_sync_pin(SyncPinState.HIGH)
    assert ot3_state.get_sync_pin_state()
    ot3_state.set_sync_pin(SyncPinState.LOW)
    assert not ot3_state.get_sync_pin_state()
