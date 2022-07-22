"""Enum containing valid pipette models."""
from enum import Enum


class PipetteModel(Enum):
    # TODO: I know these aren't the correct pipette models
    # Need to update when there is more clarity on what they are
    # Maybe pull from shared data in monorepo?
    SINGLE_20 = "P20"
    SINGLE_300 = "P300"
    MULTI_8_20 = "P20-multi-8"
    MULTI_8_300 = "P300-multi-8"
    MULTI_96_1000 = "P1000-multi-96"
