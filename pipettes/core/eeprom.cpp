#include "pipettes/core/eeprom.hpp"
#include "common/core/bit_utils.hpp"

eeprom::EEPromWriter::EEPromWriter(i2c::I2CDeviceBase & i2c_device):i2c_device{i2c_device} {}

void eeprom::EEPromWriter::transmit(uint8_t value) {
    i2c_device.master_transmit(DEVICE_ADDRESS, &value, 1, TIMEOUT);
}

void eeprom::EEPromWriter::receive(BufferType& receive) {
    i2c_device.master_receive(DEVICE_ADDRESS, receive.data(), receive.size(), TIMEOUT);
}


void eeprom::write(EEPromWriter& writer, const uint8_t serial_number) {
    writer.transmit(serial_number);
}

auto eeprom::read(EEPromWriter& writer) -> uint8_t {
    using BufferType = std::array<uint8_t, EEPromWriter::BUFFER_SIZE>;
    BufferType rxBuffer{0};
    uint8_t data = 0x0;

    writer.receive(rxBuffer);

    bit_utils::bytes_to_int(rxBuffer, data);
    return data;
}
