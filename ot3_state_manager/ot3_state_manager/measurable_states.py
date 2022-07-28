"""Contains the different states for OT3 that can be monitored."""
from dataclasses import dataclass

from opentrons.hardware_control.types import OT3Axis


@dataclass
class Position:
    """Represents position state."""

    axis: OT3Axis
    current_position: float = 0.0
    commanded_position: float = 0.0
    encoder_position: float = 0.0


@dataclass
class Force:
    """Represents force state."""

    axis: OT3Axis
    current_force: float = 0.0
    commanded_force: float = 0.0
    encoder_force: float = 0.0
