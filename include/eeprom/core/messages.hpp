#pragma once

#include "eeprom/core/hardware_iface.hpp"
#include "eeprom/core/types.hpp"
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

struct ConfigResponseMessage {
    eeprom::hardware_iface::EEPromChipType chip;
    types::address addr_bytes;
    types::data_length mem_size;
};

using ConfigRequestCallback = void (*)(const ConfigResponseMessage&, void*);

struct ConfigRequestMessage {
    ConfigRequestCallback callback;
    void* callback_param;
};

}  // namespace message
}  // namespace eeprom
