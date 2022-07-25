"""Contains the different states for OT3 that can be monitored."""
from dataclasses import dataclass

from ot3_state_manager.ot3_axis import OT3Axis


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

    current_force: float = 0.0
    commanded_force: float = 0.0
    encoder_force: float = 0.0
