/*
** Functions and definitions for deciding what kind of pipette this is.
*/
#pragma once
#include <array>
#include <cstdint>

#include "can/core/can_writer_task.hpp"
#include "can/core/ids.hpp"
#include "can/core/messages.hpp"
#include "eeprom/core/addresses.hpp"
#include "eeprom/core/serial_number.hpp"

namespace pipette_info {
using namespace can::ids;
using namespace can::messages;

// These defines are used to help parse pipette serials, which are stored in
// a structured binary + ascii format:
// +-----+--------+------+
// |0x00 |name    |binary|
// |0x01 |        |      |
// +-----+--------+------+
// |0x02 |model   |binary|
// +-----+--------+------+
// |0x04 |datecode|ascii |
// |0x13 |        |      |
// +-----+--------+------+
// The binary name is a uint16 that is a lookup into the pipette name table.
// The binary model is a uint16 that is a number. Finally, the datecode is
// ascii and is the remnant of the serial. For instance, serial
// P1KV3120210214A02 would be stored as
// name: 0x0000 (p1000_single)
// model: 0x001f (model 31)
// datecode: "20210214A02" (datecode for second unit produced on 14/02/21)
constexpr size_t PIPETTE_NAME_FIELD_START = 0;
constexpr size_t PIPETTE_NAME_FIELD_LEN = 2;
constexpr size_t PIPETTE_MODEL_FIELD_START = PIPETTE_NAME_FIELD_LEN;
constexpr size_t PIPETTE_MODEL_FIELD_LEN = 2;
constexpr size_t PIPETTE_DATACODE_START =
    PIPETTE_MODEL_FIELD_START + PIPETTE_MODEL_FIELD_LEN;
constexpr size_t PIPETTE_DATACODE_LEN =
    sizeof(eeprom::serial_number::SerialDataCodeType);

/**
 * A HandlesMessages implementing class that will respond to system messages.
 *
 * @tparam CanClient can writer task client
 * @tparam EEPromClient eeprom task client
 */
template <can::message_writer_task::TaskClient CanClient,
          eeprom::task::TaskClient EEPromClient>
class PipetteInfoMessageHandler : eeprom::accessor::ReadListener {
  public:
    /**
     * Constructor
     *
     * @param writer A message writer for sending the response
     * @param eeprom_client An eeprom task client
     */
    explicit PipetteInfoMessageHandler(CanClient &writer,
                                       EEPromClient &eeprom_client)
        : writer(writer),
          serial_number_accessor{eeprom_client, *this, sn_accessor_backing} {}
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
    void read_complete(uint32_t message_index) final {
        std::array<uint8_t, eeprom::addresses::serial_number_length> serial{};
        std::copy_n(sn_accessor_backing.begin(),
                    eeprom::addresses::serial_number_length, serial.begin());
        writer.send_can_message(
            can::ids::NodeId::host,
            can::messages::PipetteInfoResponse{
                .message_index = message_index,
                .name = get_name(sn_accessor_backing),
                .model = get_model(sn_accessor_backing),
                .serial = get_data_code(sn_accessor_backing)});
    }

  private:
    void visit(std::monostate &) {}

    /**
     * Handle a request to get instrument info.
     */
    void visit(const InstrumentInfoRequest &m) {
        // Start a serial number read. Respond with CAN message when read
        // completes.
        serial_number_accessor.start_read(m.message_index);
    }

    /**
     * Handle request to set the serial number.
     * @param m
     */
    void visit(const SetSerialNumber &m) {
        std::copy_n(m.serial.begin(), sn_accessor_backing.size(),
                    sn_accessor_backing.begin());
        serial_number_accessor.write(sn_accessor_backing);
        writer.send_can_message(can::ids::NodeId::host,
                                can::messages::ack_from_request(m));
    }

    CanClient &writer;
    eeprom::serial_number::SerialNumberType sn_accessor_backing =
        eeprom::serial_number::SerialNumberType{};
    eeprom::serial_number::SerialNumberAccessor<EEPromClient>
        serial_number_accessor;
    static auto get_model(const eeprom::serial_number::SerialNumberType &serial)
        -> uint16_t {
        uint16_t model = 0;
        const auto *iter = serial.cbegin();
        const auto *bound = serial.cbegin();
        std::advance(iter, PIPETTE_MODEL_FIELD_START);
        std::advance(bound,
                     PIPETTE_MODEL_FIELD_START + PIPETTE_MODEL_FIELD_LEN);
        iter = bit_utils::bytes_to_int(iter, bound, model);
        return model;
    }

    static auto get_name(const eeprom::serial_number::SerialNumberType &serial)
        -> uint16_t {
        uint16_t name = 0;
        const auto *iter = serial.cbegin();
        const auto *bound = serial.cbegin();
        std::advance(iter, PIPETTE_NAME_FIELD_START);
        std::advance(bound, PIPETTE_NAME_FIELD_START + PIPETTE_NAME_FIELD_LEN);
        iter = bit_utils::bytes_to_int(iter, bound, name);
        return name;
    }

    static auto get_data_code(
        const eeprom::serial_number::SerialNumberType &serial)
        -> eeprom::serial_number::SerialDataCodeType {
        eeprom::serial_number::SerialDataCodeType dc;
        std::copy_n(serial.begin() + PIPETTE_DATACODE_START,
                    PIPETTE_DATACODE_LEN, dc.begin());
        return dc;
    }
};
};  // namespace pipette_info
