"""Home for all custom errors related to OT3StateManager."""


class PipetteConfigurationError(Exception):
    """Error thrown when OT3StateManager is configured with invalid pipette config."""

    def __init__(self, msg: str) -> None:
        super().__init__(msg)


class HardwareNotAttachedError(Exception):
    """Error thrown when hardware specified is not actually attached to the OT3."""

    def __init__(self, hardware_name: str) -> None:
        super().__init__(f"Hardware, {hardware_name}, is not attached to the OT3.")


class NoPositionProvidedError(Exception):
    """Error thrown when no position data is provided to update_position method of OT3StateManager."""  # noqa: E501

    def __init__(self) -> None:
        super().__init__(
            "No position was provided for update_position. "
            "Must provide at least 1 position value."
        )
