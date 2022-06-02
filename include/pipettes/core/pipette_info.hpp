/*
** Functions and definitions for deciding what kind of pipette this is.
*/
#pragma once
#include <array>
#include <cstdint>

#include "can/core/can_writer_task.hpp"
#include "can/core/ids.hpp"
#include "can/core/messages.hpp"
#include "eeprom/core/serial_number.hpp"

namespace pipette_info {
using namespace can_ids;
using namespace can_messages;

enum class PipetteName {
    P1000_SINGLE = 0,
    P1000_MULTI = 1,
    P1000_96 = 2,
    P1000_384 = 3,
};

struct PipetteInfo {
    PipetteName name;
    uint16_t model;
    std::array<char, 12> serial;
};

// These are implemented in pipette-type-specific source files in core
// e.g. pipettes/core/pipette_type_single.cpp
PipetteName get_name();
uint16_t get_model();

/**
 * A HandlesMessages implementing class that will respond to system messages.
 *
 * @tparam CanClient can writer task client
 * @tparam EEPromClient eeprom task client
 */
template <message_writer_task::TaskClient CanClient,
          eeprom::task::TaskClient EEPromClient>
class PipetteInfoMessageHandler : eeprom::serial_number::ReadListener {
  public:
    /**
     * Constructor
     *
     * @param writer A message writer for sending the response
     * @param eeprom_client An eeprom task client
     */
    explicit PipetteInfoMessageHandler(CanClient &writer,
                                       EEPromClient &eeprom_client)
        : writer(writer), serial_number_accessor{eeprom_client, *this} {}
    PipetteInfoMessageHandler(const PipetteInfoMessageHandler &) = delete;
    PipetteInfoMessageHandler(const PipetteInfoMessageHandler &&) = delete;
    auto operator=(const PipetteInfoMessageHandler &)
        -> PipetteInfoMessageHandler & = delete;
    auto operator=(const PipetteInfoMessageHandler &&)
        -> PipetteInfoMessageHandler && = delete;
    ~PipetteInfoMessageHandler() = default;

    using MessageType =
        std::variant<std::monostate, InstrumentInfoRequest, SetSerialNumber>;

    /**
     * Message handler
     * @param m The incoming message.
     */
    void handle(MessageType &m) {
        std::visit([this](auto o) { this->visit(o); }, m);
    }

    /**
     * A serial number read has completed.
     * @param sn The serial number.
     */
    void on_read(const eeprom::serial_number::SerialNumberType &sn) final {
        writer.send_can_message(can_ids::NodeId::host,
                                can_messages::PipetteInfoResponse{
                                    .name = static_cast<uint16_t>(get_name()),
                                    .model = get_model(),
                                    .serial = sn});
    }

  private:
    void visit(std::monostate &) {}

    /**
     * Handle a request to get instrument info.
     */
    void visit(const InstrumentInfoRequest &) {
        // Start a serial number read. Respond with CAN message when read
        // completes.
        serial_number_accessor.start_read();
    }

    /**
     * Handle request to set the serial number.
     * @param m
     */
    void visit(const SetSerialNumber &m) {
        serial_number_accessor.write(m.serial);
    }

    CanClient &writer;
    eeprom::serial_number::SerialNumberAccessor<EEPromClient>
        serial_number_accessor;
};
};  // namespace pipette_info
