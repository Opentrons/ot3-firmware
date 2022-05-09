#pragma once

namespace eeprom {
namespace message {

/** Eeprom message */
struct EepromMessage {
    eeprom::types::address memory_address;
    eeprom::types::data_length length;
    eeprom::types::EepromData data;

    auto operator==(const EepromMessage&) const -> bool = default;
};

using ReadResponseCallback = void (*)(const EepromMessage&, void*);

/**
 * The read from eeprom message.
 */
struct ReadEepromMessage {
    eeprom::types::address memory_address;
    eeprom::types::data_length length;
    ReadResponseCallback callback;
    void* callback_param;
};

/** The write to eeprom message */
using WriteEepromMessage = EepromMessage;

}  // namespace message
}  // namespace eeprom