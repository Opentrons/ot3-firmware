"""Contains the different states for OT3 that can be monitored."""
from dataclasses import dataclass

from opentrons.hardware_control.types import Axis


@dataclass
class Position:
    """Represents position state."""

    axis: Axis
    current_position: int = 0
    commanded_position: int = 0
    encoder_position: int = 0


@dataclass
class Force:
    """Represents force state."""

    axis: Axis
    current_force: int = 0
    commanded_force: int = 0
    encoder_force: int = 0
