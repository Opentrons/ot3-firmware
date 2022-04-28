#pragma once

namespace eeprom {
namespace message {

/** Basic eeprom message */
struct EepromMessage {
    eeprom::types::address memory_address{0};
    eeprom::types::data_length length{0};
    eeprom::types::EepromData data{};
};

/** Forward declaration of response handler interface */
class EepromResponseHandler;

/**
 * The read from eeprom message.
 */
struct ReadEepromMessage : public EepromMessage {
    EepromResponseHandler* response_handler{nullptr};
};

/** The write to eeprom message */
using WriteEepromMessage = EepromMessage;

/**
 * The eeprom response handler interface
 */
class EepromResponseHandler {
  public:
    virtual void handle(const EepromMessage& m) {}
    virtual ~EepromResponseHandler(){}
};


}  // namespace message
}  // namespace eeprom