"""Enum containing valid pipette models."""
from __future__ import annotations

from enum import Enum


class PipetteModel(Enum):
    """Enum containing valid pipette models for OT3."""

    # TODO: I know these aren't the correct pipette models
    # Need to update when there is more clarity on what they are
    # Maybe pull from shared data in monorepo?
    SINGLE_20 = "P20"
    SINGLE_300 = "P300"
    MULTI_8_20 = "P20-multi-8"
    MULTI_8_300 = "P300-multi-8"
    MULTI_96_1000 = "P1000-multi-96"

    @staticmethod
    def model_name_to_enum(name: str) -> PipetteModel:
        """Get PipetteModel by name of pipette."""
        lookup_dict = {
            PipetteModel.SINGLE_20.value: PipetteModel.SINGLE_20,
            PipetteModel.SINGLE_300.value: PipetteModel.SINGLE_300,
            PipetteModel.MULTI_8_20.value: PipetteModel.MULTI_8_20,
            PipetteModel.MULTI_8_300.value: PipetteModel.MULTI_8_300,
            PipetteModel.MULTI_96_1000.value: PipetteModel.MULTI_96_1000,
        }
        return lookup_dict[name]

    def get_pipette_name(self) -> str:
        """Get name of pipette.

        This is the same as calling .value. But .value is not typed to a string and
        this method is.
        """
        return str(self.value)
