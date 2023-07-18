"""Class to maintain state of the OT3."""

from __future__ import annotations

import logging
from typing import Dict, List, Optional

from opentrons.hardware_control.types import Axis

from .hardware import (
    GantryX,
    GantryY,
    Gripper,
    Head,
    LeftPipette,
    RightPipette,
    SyncPin,
)
from .measurable_states import Position
from .pipette_model import PipetteModel
from .util import Direction, SyncPinState

log = logging.getLogger(__name__)


class OT3State:
    """Class to maintain state of OT3.

    Provides properties to get current_position, commanded_position, and
    encoder_position. Also provides update_position function to update the previously
    state values.
    """

    @classmethod
    def build(
        cls,
        left_pipette_model_name: Optional[str],
        right_pipette_model_name: Optional[str],
        use_gripper: bool,
    ) -> OT3State:
        """Helper function to build an OT3State object.

        Takes optional left pipette and right pipette model names. Passing None for the
        pipette models names tells the system that there is no pipette attached that
        slot.

        Pass True for use_gripper parameter to tell the model there is a gripper
        attached. Pass False if no gripper is attached.

        If you are attaching a high-throughput pipette it must go in the right-side
        slot.
        """
        left_pipette = (
            LeftPipette(PipetteModel.model_name_to_enum(left_pipette_model_name))
            if left_pipette_model_name is not None
            else None
        )
        right_pipette = (
            RightPipette(PipetteModel.model_name_to_enum(right_pipette_model_name))
            if right_pipette_model_name is not None
            else None
        )
        gripper = Gripper() if use_gripper else None

        return cls(
            left_pipette=left_pipette, right_pipette=right_pipette, gripper=gripper
        )

    def __init__(
        self,
        left_pipette: Optional[LeftPipette],
        right_pipette: Optional[RightPipette],
        gripper: Optional[Gripper],
    ) -> None:
        """Creates OT3State object.

        Args:
            left_pipette: PipetteModel to attach, or None.
            right_pipette: PipetteModel to attach, or None.
            gripper: Whether or not to attach a gripper.
        """
        if (
            left_pipette is not None
            and left_pipette.model == PipetteModel.MULTI_96_1000
        ):
            raise ValueError(
                "High-throughput pipette must be connected to right pipette mount"
            )

        if (
            left_pipette is not None
            and right_pipette is not None
            and right_pipette.model == PipetteModel.MULTI_96_1000
        ):
            raise ValueError(
                "When using high-throughput pipette you cannot have a pipette in the "
                "left mount"
            )

        self.head = Head()
        self.gantry_x = GantryX()
        self.gantry_y = GantryY()
        self.left_pipette = left_pipette
        self.right_pipette = right_pipette
        self.gripper = gripper
        self.sync_pin = SyncPin()

    def _pos_dict(self) -> Dict[Axis, Optional[Position]]:
        """Generates lookup dict for class's position properties."""
        left_pipette_pos = (
            self.left_pipette.position if self.left_pipette is not None else None
        )
        right_pipette_pos = (
            self.right_pipette.position if self.right_pipette is not None else None
        )
        gripper_pos = self.gripper.position if self.gripper is not None else None
        gripper_mount_pos = (
            self.gripper.gripper_z_position if self.gripper is not None else None
        )

        return {
            Axis.Z_L: self.head.left_pipette_z_position,
            Axis.Z_R: self.head.right_pipette_z_position,
            Axis.X: self.gantry_x.position,
            Axis.Y: self.gantry_y.position,
            Axis.P_L: left_pipette_pos,
            Axis.P_R: right_pipette_pos,
            Axis.G: gripper_pos,
            Axis.Z_G: gripper_mount_pos,
        }

    @property
    def current_position(self) -> Dict[Axis, Optional[int]]:
        """Returns dictionary with axis mapped to current position of said axis.

        If hardware is not attached for axis, then position value will be None.
        """
        pos_dict = self._pos_dict()

        # Ignoring the mypy error because we are checking that the value is None
        return {
            key: (pos_dict[key].current_position if value is not None else None)  # type: ignore[union-attr]
            for key, value in pos_dict.items()
        }

    @property
    def encoder_position(self) -> Dict[Axis, Optional[int]]:
        """Returns dictionary with axis mapped to encoder position of said axis.

        If hardware is not attached for axis, then position value will be None.
        """
        pos_dict = self._pos_dict()

        # Ignoring the mypy error because we are checking that the value is None
        return {
            key: (pos_dict[key].encoder_position if value is not None else None)  # type: ignore[union-attr]
            for key, value in pos_dict.items()
        }

    @property
    def updatable_axes(self) -> List[Axis]:
        """Returns a list of positions that are available to be updated."""
        return [key for key, value in self._pos_dict().items() if value is not None]

    @property
    def is_gripper_attached(self) -> bool:
        """Returns True if gripper is attached, False if not."""
        return self.gripper is not None

    @property
    def is_left_pipette_attached(self) -> bool:
        """Returns True if left pipette is attached, False if not."""
        return self.left_pipette is not None

    @property
    def is_right_pipette_attached(self) -> bool:
        """Returns True if right pipette is attached, False if not."""
        return self.right_pipette is not None

    @property
    def is_high_throughput_pipette_attached(self) -> bool:
        """Returns True if high throughput pipette is attached, False if not."""
        return (
            self.right_pipette is not None
            and self.right_pipette.model == PipetteModel.MULTI_96_1000
        )

    def axis_current_position(self, axis: Axis) -> Optional[int]:
        """Returns current position or None for specified axis."""
        return self.current_position[axis]

    def axis_encoder_position(self, axis: Axis) -> Optional[int]:
        """Returns encoder position or None for specified axis."""
        return self.encoder_position[axis]

    def update_position(
        self,
        axis_to_update: Axis,
        current_position: int,
        encoder_position: int,
    ) -> None:
        """Method to update the position of a piece of OT3 hardware."""
        pos_to_update = self._pos_dict()[axis_to_update]

        if pos_to_update is None:
            raise ValueError(
                f'Axis "{axis_to_update.name}" is not available. '
                f"Cannot update it's position."
            )
        pos_to_update.current_position = current_position
        pos_to_update.encoder_position = encoder_position

    def pulse(self, axis: Axis, direction: Direction) -> None:
        """Increments or decrements current and encoder position by 1 for axis."""
        log.info(f"SERVER: Pulsing {axis}")
        mod = 1 if direction == Direction.POSITIVE else -1

        axis_current_position = self.axis_current_position(axis)
        axis_encoder_position = self.axis_encoder_position(axis)

        if axis_current_position is None or axis_encoder_position is None:
            raise ValueError("Trying to pulse axis that is not currently configured.")

        self.update_position(
            axis_to_update=axis,
            current_position=axis_current_position + mod,
            encoder_position=axis_encoder_position + mod,
        )

    def set_sync_pin(self, state: SyncPinState) -> None:
        """Sets sync pin high or low based off of state passed."""
        if state is SyncPinState.HIGH:
            self.sync_pin.set_sync_pin_high()
        else:
            self.sync_pin.set_sync_pin_low()

    def get_sync_pin_state(self) -> bool:
        """Returns the sync pin state."""
        return self.sync_pin.sync_pin
