/*
** Functions and definitions for deciding what kind of hepauv this is.
*/
#pragma once
#include <array>
#include <cstdint>

#include "can/core/can_writer_task.hpp"
#include "can/core/ids.hpp"
#include "can/core/messages.hpp"
#include "eeprom/core/serial_number.hpp"

namespace hepauv_info {
using namespace can::ids;
using namespace can::messages;

// These defines are used to help parse hepauv serials
constexpr size_t HEPAUV_MODEL_FIELD_START = 0;
constexpr size_t HEPAUV_MODEL_FIELD_LEN = 2;
constexpr size_t HEPAUV_DATACODE_START =
    HEPAUV_MODEL_FIELD_START + HEPAUV_MODEL_FIELD_LEN;
constexpr size_t HEPAUV_DATACODE_LEN =
    sizeof(eeprom::serial_number::SerialDataCodeType);

/**
 * A HandlesMessages implementing class that will respond to system messages.
 *
 * @tparam CanClient can writer task client
 * @tparam EEPromClient eeprom task client
 */
template <can::message_writer_task::TaskClient CanClient,
          eeprom::task::TaskClient EEPromClient>
class HepaUVInfoMessageHandler : eeprom::accessor::ReadListener {
  public:
    /**
     * Constructor
     *
     * @param writer A message writer for sending the response
     * @param eeprom_client An eeprom task client
     */
    explicit HepaUVInfoMessageHandler(CanClient &writer,
                                      EEPromClient &eeprom_client)
        : writer{writer},
          serial_number_accessor{eeprom_client, *this, sn_accessor_backing} {}
    HepaUVInfoMessageHandler(const HepaUVInfoMessageHandler &) = delete;
    HepaUVInfoMessageHandler(const HepaUVInfoMessageHandler &&) = delete;
    auto operator=(const HepaUVInfoMessageHandler &)
        -> HepaUVInfoMessageHandler & = delete;
    auto operator=(const HepaUVInfoMessageHandler &&)
        -> HepaUVInfoMessageHandler && = delete;
    ~HepaUVInfoMessageHandler() final = default;

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
     * @param sn Serial number
     */
    void read_complete(uint32_t message_index) final {
        std::array<uint8_t, eeprom::addresses::serial_number_length> serial{};
        std::copy_n(sn_accessor_backing.begin(),
                    eeprom::addresses::serial_number_length, serial.begin());
        writer.send_can_message(
            can::ids::NodeId::host,
            HepaUVInfoResponse{
                .message_index = message_index,
                .model = get_hepauv_model(sn_accessor_backing),
                .serial = get_hepauv_data_code(sn_accessor_backing)});
    }

  private:
    void visit(std::monostate &) {}

    /**
     * Handle request for instrument info.
     */
    void visit(const InstrumentInfoRequest &m) {
        // Start a serial number read. Respond with CAN message when read
        // completes.
        serial_number_accessor.start_read(m.message_index);
    }

    /**
     * Handle request to set the serial number.
     * @param m The message
     */
    void visit(const SetSerialNumber &m) {
        std::copy_n(m.serial.begin(), sn_accessor_backing.size(),
                    sn_accessor_backing.begin());
        serial_number_accessor.write(sn_accessor_backing, m.message_index);
        writer.send_can_message(can::ids::NodeId::host,
                                can::messages::ack_from_request(m));
    }

    CanClient &writer;
    eeprom::serial_number::SerialNumberType sn_accessor_backing =
        eeprom::serial_number::SerialNumberType{};
    eeprom::serial_number::SerialNumberAccessor<EEPromClient>
        serial_number_accessor;

    static auto get_hepauv_model(
        const eeprom::serial_number::SerialNumberType &serial) -> uint16_t {
        uint16_t model = 0;
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        const auto *iter = serial.begin() + HEPAUV_MODEL_FIELD_START;
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        iter =
            bit_utils::bytes_to_int(iter, iter + HEPAUV_MODEL_FIELD_LEN, model);
        return model;
    }

    static auto get_hepauv_data_code(
        const eeprom::serial_number::SerialNumberType &serial)
        -> eeprom::serial_number::SerialDataCodeType {
        eeprom::serial_number::SerialDataCodeType dc;
        std::copy_n(serial.begin() + HEPAUV_DATACODE_START, HEPAUV_DATACODE_LEN,
                    dc.begin());
        return dc;
    }
};
};  // namespace hepauv_info
