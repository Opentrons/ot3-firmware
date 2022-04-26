#pragma once


namespace eeprom {
namespace message {


/**
 *
 */
struct EepromMessage {
    address memory_address{0};
    data_length length{0};
    EepromData data{};
};

/**
 *
 */
struct ReadEepromMessage : public EepromMessage {
    ResponseHandler * response_handler{nullptr};
};

/**
 *
 */
class ResponseHandler {
  public:
    virtual void handle_eeprom_response(const EepromMessage& m) {}
};


using WriteEepromMessage = EepromMessage;

}
}