#pragma once

namespace eeprom {
namespace message {

/** Eeprom message */
struct EepromMessage {
    eeprom::types::address memory_address{0};
    eeprom::types::data_length length{0};
    eeprom::types::EepromData data{};

    auto operator==(const EepromMessage&) const -> bool = default;
};

using ReadResponseCallback = void (*)(const EepromMessage&, void*);

/**
 * The read from eeprom message.
 */
struct ReadEepromMessage {
    eeprom::types::address memory_address{0};
    eeprom::types::data_length length{0};
    ReadResponseCallback callback;
    void* callback_param;
};

/** The write to eeprom message */
using WriteEepromMessage = EepromMessage;

}  // namespace message
}  // namespace eeprom