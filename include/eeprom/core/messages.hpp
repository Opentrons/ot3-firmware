#pragma once

#include "eeprom/core/types.hpp"
#include "eeprom/core/hardware_iface.hpp"
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
    eeprom::hardware_iface::EEPromAddressType addr_bytes;
    eeprom::hardware_iface::EEpromMemorySize mem_size;
};

using ConifgRequestCallback = void (*)(const ConfigResponseMessage&, void*);

struct ConfigRequestMessage {
    ConifgRequestCallback callback;
    void* callback_param;
};
    


}  // namespace message
}  // namespace eeprom
