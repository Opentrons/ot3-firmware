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

/** Forward declaration of response handler interface */
class EepromResponseHandler;

/**
 * The read from eeprom message.
 */
struct ReadEepromMessage {
    eeprom::types::address memory_address{0};
    eeprom::types::data_length length{0};
    EepromResponseHandler* response_handler{nullptr};
};

/** The write to eeprom message */
using WriteEepromMessage = EepromMessage;

/**
 * The eeprom response handler interface
 */
class EepromResponseHandler {
  public:
    virtual void handle(const EepromMessage& m) = 0;
    virtual ~EepromResponseHandler() {}
};

}  // namespace message
}  // namespace eeprom