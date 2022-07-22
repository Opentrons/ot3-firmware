class PipetteConfigurationError(Exception):
    def __init__(self, msg: str) -> None:
        super().__init__(msg)


class HardwareNotAttachedError(Exception):
    def __init__(self, hardware_name: str) -> None:
        super().__init__(f"Hardware, {hardware_name}, is not attached to the OT3.")


class NoPositionProvidedError(Exception):
    def __init__(self) -> None:
        super().__init__(
            "No position was provided for update_position. "
            "Must provide at least 1 position value."
        )
