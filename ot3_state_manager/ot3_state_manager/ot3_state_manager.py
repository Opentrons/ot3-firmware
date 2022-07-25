"""Class to maintain state of the OT3."""

from __future__ import annotations

from typing import Dict, List, Optional

from ot3_state_manager.hardware import (
    GantryX,
    GantryY,
    Gripper,
    Hardware,
    Head,
    LeftPipette,
    RightPipette,
    UpdatablePositions,
)
from ot3_state_manager.measurable_states import Position
from ot3_state_manager.pipette_model import PipetteModel


class OT3StateManager:
    """Class to maintain state of OT3.

    Provides properties to get current_position, commanded_position, and
    encoder_position. Also provides update_position function to update the previously
    state values.
    """

    @classmethod
    def build(
        cls,
        left_pipette_model: Optional[PipetteModel],
        right_pipette_model: Optional[PipetteModel],
        use_gripper: bool,
    ) -> OT3StateManager:
        """Helper function to build an OT3StateManager object.

        Takes optional left pipette and right pipette model names. Passing None for the
        pipette models names tells the system that there is no pipette attached that
        slot.

        Pass True for use_gripper parameter to tell the model there is a gripper
        attached. Pass False if no gripper is attached.

        If you are attaching a high-throughput pipette it must go in the right-side
        slot.
        """
        left_pipette = (
            LeftPipette(model=left_pipette_model)
            if left_pipette_model is not None
            else None
        )
        right_pipette = (
            RightPipette(model=right_pipette_model)
            if right_pipette_model is not None
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
        """Creates OT3StateManager object.

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

    def _pos_dict(self) -> Dict[str, Optional[Position]]:
        """Generates lookup dict for class's position properties."""
        left_pipette_pos = (
            self.left_pipette.position if self.left_pipette is not None else None
        )
        right_pipette_pos = (
            self.right_pipette.position if self.right_pipette is not None else None
        )
        gripper_pos = self.gripper.position if self.gripper is not None else None

        return {
            Head.gripper_mount_position.axis.name: self.head.gripper_mount_position,
            Head.left_pipette_mount_position.axis.name: self.head.left_pipette_mount_position,
            Head.right_pipette_mount_position.axis.name: self.head.right_pipette_mount_position,
            GantryX.position.axis.name: self.gantry_x.position,
            GantryY.position.axis.name: self.gantry_y.position,
            LeftPipette.position.axis.name: left_pipette_pos,
            RightPipette.position.axis.name: right_pipette_pos,
            Gripper.position.axis.name: gripper_pos,
        }

    @property
    def current_position(self) -> Dict[str, Optional[float]]:
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
    def commanded_position(self) -> Dict[str, Optional[float]]:
        """Returns dictionary with axis mapped to commanded position of said axis.

        If hardware is not attached for axis, then position value will be None.
        """
        pos_dict = self._pos_dict()

        # Ignoring the mypy error because we are checking that the value is None
        return {
            key: (pos_dict[key].commanded_position if value is not None else None)  # type: ignore[union-attr]
            for key, value in pos_dict.items()
        }

    @property
    def encoder_position(self) -> Dict[str, Optional[float]]:
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
    def updatable_positions(self) -> List[UpdatablePositions]:
        """Returns a list of positions that are available to be updated."""
        return [
            key
            for key, value in self._updatable_pos_to_hardware_dict.items()
            if value is not None
        ]

    @property
    def _updatable_pos_to_hardware_dict(
        self,
    ) -> Dict[UpdatablePositions, Optional[Hardware]]:

        return {
            UpdatablePositions.GANTRY_X: self.gantry_x,
            UpdatablePositions.GANTRY_Y: self.gantry_y,
            UpdatablePositions.GRIPPER_MOUNT: self.head,
            UpdatablePositions.LEFT_PIPETTE_MOUNT: self.head,
            UpdatablePositions.RIGHT_PIPETTE_MOUNT: self.head,
            UpdatablePositions.GRIPPER_EXTEND: (
                self.gripper if self.is_gripper_attached else None
            ),
            UpdatablePositions.LEFT_PIPETTE_PLUNGER: (
                self.left_pipette if self.is_left_pipette_attached else None
            ),
            UpdatablePositions.RIGHT_PIPETTE_PLUNGER: (
                self.right_pipette if self.is_right_pipette_attached else None
            ),
        }

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

    @staticmethod
    def _update_pos(
        hardware: Hardware,
        current_position: Optional[float],
        commanded_position: Optional[float],
        encoder_position: Optional[float],
    ) -> None:

        if current_position is not None:
            hardware.position.current_position = current_position
        if commanded_position is not None:
            hardware.position.commanded_position = commanded_position
        if encoder_position is not None:
            hardware.position.encoder_position = encoder_position

    def update_position(
        self,
        pos_to_update: UpdatablePositions,
        current_position: Optional[float],
        commanded_position: Optional[float],
        encoder_position: Optional[float],
    ) -> None:
        """Method to update the position of a piece of OT3 hardware.

        Pass an UpdatablePosition object and at least 1 position to update.

        Throws exception if no positions are provided and if the underlying hardware
        related to UpdatablePosition is not attached.
        """
        if all(
            [
                pos is None
                for pos in (current_position, commanded_position, encoder_position)
            ]
        ):
            raise ValueError("You must provide a least one position to update.")

        hardware_to_update = self._updatable_pos_to_hardware_dict[pos_to_update]

        if hardware_to_update is None:
            raise ValueError(
                f'Hardware "{pos_to_update.value}" is not attached. '
                f"Cannot update it's position."
            )

        self._update_pos(
            hardware_to_update, current_position, commanded_position, encoder_position
        )
