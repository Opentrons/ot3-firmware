"""Payloads of can bus messages."""
# TODO (amit, 2022-01-26): Figure out why using annotations import ruins
#  dataclass fields interpretation.
#  from __future__ import annotations
from dataclasses import dataclass

from .. import utils


@dataclass
class EmptyPayload(utils.BinarySerializable):
    """An empty payload."""

    pass


@dataclass
class DeviceInfoResponsePayload(utils.BinarySerializable):
    """Device info response."""

    version: utils.UInt32Field


@dataclass
class GetStatusResponsePayload(utils.BinarySerializable):
    """Get status response."""

    status: utils.UInt8Field
    data: utils.UInt32Field


@dataclass
class MoveRequestPayload(utils.BinarySerializable):
    """Move request."""

    steps: utils.UInt32Field


@dataclass
class GetSpeedResponsePayload(utils.BinarySerializable):
    """Get speed response."""

    mm_sec: utils.UInt32Field


@dataclass
class WriteToEEPromRequestPayload(utils.BinarySerializable):
    """Write to eeprom request."""

    serial_number: utils.UInt16Field


@dataclass
class ReadFromEEPromResponsePayload(utils.BinarySerializable):
    """Read from ee prom response."""

    serial_number: utils.UInt16Field


@dataclass
class MoveGroupRequestPayload(utils.BinarySerializable):
    """A payload with a group id."""

    group_id: utils.UInt8Field


@dataclass
class MoveGroupResponsePayload(utils.BinarySerializable):
    """A response payload with a group id."""

    group_id: utils.UInt8Field


@dataclass
class AddToMoveGroupRequestPayload(MoveGroupRequestPayload):
    """Base of add to move group request to a message group."""

    seq_id: utils.UInt8Field
    duration: utils.UInt32Field


@dataclass
class AddLinearMoveRequestPayload(AddToMoveGroupRequestPayload):
    """Add a linear move request to a message group."""

    request_stop_condition: utils.UInt8Field
    acceleration: utils.Int32Field
    velocity: utils.Int32Field


@dataclass
class GetMoveGroupResponsePayload(MoveGroupResponsePayload):
    """Response to request to get a move group."""

    num_moves: utils.UInt8Field
    total_duration: utils.UInt32Field


@dataclass
class ExecuteMoveGroupRequestPayload(MoveGroupRequestPayload):
    """Start executing a move group."""

    start_trigger: utils.UInt8Field
    cancel_trigger: utils.UInt8Field


@dataclass
class MoveCompletedPayload(MoveGroupResponsePayload):
    """Notification of a completed move group."""

    seq_id: utils.UInt8Field
    current_position: utils.UInt32Field
    ack_id: utils.UInt8Field


@dataclass
class MotionConstraintsPayload(utils.BinarySerializable):
    """The min and max velocity and acceleration of a motion system."""

    min_velocity: utils.Int32Field
    max_velocity: utils.Int32Field
    min_acceleration: utils.Int32Field
    max_acceleration: utils.Int32Field


@dataclass
class MotorDriverRegisterPayload(utils.BinarySerializable):
    """Read motor driver register request payload."""

    reg_addr: utils.UInt8Field


@dataclass
class MotorDriverRegisterDataPayload(MotorDriverRegisterPayload):
    """Write motor driver register request payload."""

    data: utils.UInt32Field


@dataclass
class ReadMotorDriverRegisterResponsePayload(utils.BinarySerializable):
    """Read motor driver register response payload."""

    reg_addr: utils.UInt8Field
    data: utils.UInt32Field


@dataclass
class ReadPresenceSensingVoltageResponsePayload(utils.BinarySerializable):
    """Read head presence sensing voltage response payload."""

    # All values in millivolts
    z_motor: utils.UInt16Field
    a_motor: utils.UInt16Field
    gripper: utils.UInt16Field


@dataclass
class ToolsDetectedNotificationPayload(utils.BinarySerializable):
    """Tool detection notification."""

    # Tools are mapped to an enum
    z_motor: utils.UInt8Field
    a_motor: utils.UInt8Field
    gripper: utils.UInt8Field


@dataclass
class FirmwareUpdateWithAddress(utils.BinarySerializable):
    """A FW update payload with an address."""

    address: utils.UInt32Field


class FirmwareUpdateDataField(utils.BinaryFieldBase[bytes]):
    """The data field of FirmwareUpdateData."""

    NUM_BYTES = 56
    FORMAT = f"{NUM_BYTES}s"


@dataclass
class FirmwareUpdateData(FirmwareUpdateWithAddress):
    """A FW update data payload."""

    num_bytes: utils.UInt8Field
    reserved: utils.UInt8Field
    data: FirmwareUpdateDataField
    checksum: utils.UInt16Field

    def __post_init__(self) -> None:
        """Post init processing."""
        data_length = len(self.data.value)
        if data_length > FirmwareUpdateDataField.NUM_BYTES:
            raise ValueError(
                f"Data cannot be more than"
                f" {FirmwareUpdateDataField.NUM_BYTES} bytes."
            )

    @classmethod
    def create(cls, address: int, data: bytes) -> "FirmwareUpdateData":
        """Create a firmware update data payload."""
        checksum = 0
        obj = FirmwareUpdateData(
            address=utils.UInt32Field(address),
            num_bytes=utils.UInt8Field(len(data)),
            reserved=utils.UInt8Field(0),
            data=FirmwareUpdateDataField(data),
            checksum=utils.UInt16Field(checksum),
        )
        checksum = (1 + ~sum(obj.serialize())) & 0xFFFF
        obj.checksum.value = checksum
        return obj


@dataclass
class FirmwareUpdateDataAcknowledge(FirmwareUpdateWithAddress):
    """A FW update data acknowledge payload."""

    error_code: utils.UInt16Field


@dataclass
class FirmwareUpdateComplete(utils.BinarySerializable):
    """All data messages have been transmitted."""

    num_messages: utils.UInt32Field


@dataclass
class FirmwareUpdateCompleteAcknowledge(utils.BinarySerializable):
    """A response to the FirmwareUpdateComplete message."""

    error_code: utils.UInt16Field


@dataclass
class FirmwareUpdateStatus(utils.BinarySerializable):
    """A response to the FirmwareUpdateStatusRequest message."""

    flags: utils.UInt32Field


@dataclass
class GetLimitSwitchResponse(utils.BinarySerializable):
    """A response to the Limit Switch Status request payload."""

    switch_status: utils.UInt8Field


@dataclass
class ReadFromSensorRequest(utils.BinarySerializable):
    """Take a single reading from a sensor request payload."""

    sensor: utils.UInt8Field


@dataclass
class WriteToSensorRequest(utils.BinarySerializable):
    """Write a piece of data to a sensor request payload."""

    sensor: utils.UInt8Field
    data: utils.UInt16Field


@dataclass
class BaselineSensorRequest(utils.BinarySerializable):
    """Take a specified amount of readings from a sensor request payload."""

    sensor: utils.UInt8Field
    sample_rate: utils.UInt8Field


@dataclass
class ReadFromSensorResponse(utils.BinarySerializable):
    """A response for either a single reading or an averaged reading of a sensor."""

    sensor: utils.UInt8Field
    sensor_data: utils.UInt32Field


@dataclass
class SetSensorThresholdRequest(utils.BinarySerializable):
    """A request to set the threshold value of a sensor."""

    sensor: utils.UInt8Field
    threshold: utils.UInt32Field


@dataclass
class SensorThresholdResponse(utils.BinarySerializable):
    """A response that sends back the current threshold value of the sensor."""

    sensor: utils.UInt8Field
    threshold: utils.UInt32Field
