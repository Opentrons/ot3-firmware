# opentrons_ot3_firmware - python bindings for OT3 firmware elements

This package is static data and routines for serializing and unserializing canbus messages, as well as generating
c++ headers for those messages.

## How to define a new message

### 1. Create message id
Add unique ID to `MessageId` enum in `constants.py`. For example, `my_message_id`.

### 2. Define Payload

**Skip this step if the message does not have a data payload.**

Define the type in `messages/payloads.py`. The payload **must** be a `dataclass` derived from `utils.BinarySerializable`. Each field in the payload must be a `BinaryFieldBase` type. For convenience there are several defined for common types (`UIntXXField`, `IntXXField` where XX is the bit width and U stands for unsigned).

### 3. Define Message

Create a `dataclass` in `messages/message_definitions.py`.

The message definition has three attributes:

- `payload` - Is the data portion of the message as defined in step 2 or use `EmptyMessage` if the payload is empty.
- `payload_type` - Is the type of the parameter in `payload`. It is typed `Type[
BinarySerializable
]` and must be statically initialized.
- `message_id` - Is the message id defined in step 1. It must be typed `Literal[
MessageId.my_message_id]` and statically initialized.

### 4. Add new message to registry

Add the message type defined in step 3 to `MessageDefinition` in `messages/messages.py`.
