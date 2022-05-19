/*
** Functions and definitions for deciding what kind of gripper this is.
*/
#pragma once
#include <array>
#include <cstdint>

#include "can/core/can_writer_task.hpp"
#include "can/core/ids.hpp"
#include "can/core/messages.hpp"
#include "eeprom/core/serial_number.hpp"

namespace gripper_info {
using namespace can_ids;
using namespace can_messages;

/**
 * A HandlesMessages implementing class that will respond to system messages.
 *
 * @tparam CanClient can writer task client
 * @tparam EEPromClient eeprom task client
 */
template <message_writer_task::TaskClient CanClient,
          eeprom::task::TaskClient EEPromClient>
class GripperInfoMessageHandler : eeprom::serial_number::ReadListener {
  public:
    /**
     * Constructor
     *
     * @param writer A message writer for sending the response
     * @param eeprom_client An eeprom task client
     */
    explicit GripperInfoMessageHandler(CanClient &writer,
                                       EEPromClient &eeprom_client)
        : writer{writer}, serial_number_accessor{eeprom_client, *this} {}
    GripperInfoMessageHandler(const GripperInfoMessageHandler &) = delete;
    GripperInfoMessageHandler(const GripperInfoMessageHandler &&) = delete;
    auto operator=(const GripperInfoMessageHandler &)
        -> GripperInfoMessageHandler & = delete;
    auto operator=(const GripperInfoMessageHandler &&)
        -> GripperInfoMessageHandler && = delete;
    ~GripperInfoMessageHandler() final = default;

    using MessageType =
        std::variant<std::monostate, GripperInfoRequest, SetSerialNumber>;

    /**
     * Message handler
     * @param m The incoming message.
     */
    void handle(MessageType &m) {
        std::visit([this](auto o) { this->visit(o); }, m);
    }

    /**
     * A serial number read has completed.
     * @param sn Serial number
     */
    void on_read(const eeprom::serial_number::SerialNumberType &sn) final {
        writer.send_can_message(can_ids::NodeId::host,
                                GripperInfoResponse{.model = 1, .serial = sn});
    }

  private:
    void visit(std::monostate &) {}

    void visit(GripperInfoRequest &) { serial_number_accessor.start_read(); }

    CanClient &writer;
    eeprom::serial_number::SerialNumberAccessor<EEPromClient>
        serial_number_accessor;
};
};  // namespace gripper_info
