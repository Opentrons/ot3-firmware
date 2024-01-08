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
using namespace can::ids;
using namespace can::messages;

// These defines are used to help parse gripper serials
// A full Serial number for gripper hasn't been defined yet so we will assume
// it has a similar structure to pipettes just without the name field
// see include/pipepttes/core/pipette_info.hpps
constexpr size_t GRIPPER_MODEL_FIELD_START = 0;
constexpr size_t GRIPPER_MODEL_FIELD_LEN = 2;
constexpr size_t GRIPPER_DATACODE_START =
    GRIPPER_MODEL_FIELD_START + GRIPPER_MODEL_FIELD_LEN;
constexpr size_t GRIPPER_DATACODE_LEN =
    sizeof(eeprom::serial_number::SerialDataCodeType);

/**
 * A HandlesMessages implementing class that will respond to system messages.
 *
 * @tparam CanClient can writer task client
 * @tparam EEPromClient eeprom task client
 */
template <can::message_writer_task::TaskClient CanClient,
          eeprom::task::TaskClient EEPromClient>
class GripperInfoMessageHandler : eeprom::accessor::ReadListener {
  public:
    /**
     * Constructor
     *
     * @param writer A message writer for sending the response
     * @param eeprom_client An eeprom task client
     */
    explicit GripperInfoMessageHandler(CanClient &writer,
                                       EEPromClient &eeprom_client)
        : writer{writer},
          serial_number_accessor{eeprom_client, *this, sn_accessor_backing} {}
    GripperInfoMessageHandler(const GripperInfoMessageHandler &) = delete;
    GripperInfoMessageHandler(const GripperInfoMessageHandler &&) = delete;
    auto operator=(const GripperInfoMessageHandler &)
        -> GripperInfoMessageHandler & = delete;
    auto operator=(const GripperInfoMessageHandler &&)
        -> GripperInfoMessageHandler && = delete;
    ~GripperInfoMessageHandler() final = default;

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
        // TODO (al, 2022-05-19): Define model.
        std::array<uint8_t, eeprom::addresses::serial_number_length> serial{};
        std::copy_n(sn_accessor_backing.begin(),
                    eeprom::addresses::serial_number_length, serial.begin());
        writer.send_can_message(
            can::ids::NodeId::host,
            GripperInfoResponse{
                .message_index = message_index,
                .model = get_gripper_model(sn_accessor_backing),
                .serial = get_gripper_data_code(sn_accessor_backing)});
    }

  private:
    void visit(std::monostate &) {}

    /**
     * Handle request for instrument info.
     */
    void visit(const InstrumentInfoRequest &m) {
        // Start a serial number read. Respond with CAN message when read
        // completes.

#if PCBA_PRIMARY_REVISION == 'b'
        // this temporarliy needs to be hardcoded because the rev1 gripper
        // cannot read/write to the eeprom due to electrical issues with the
        // chip having the same address as a sensor on the i2c line for testing
        // we will unseat the eeprom chip and use this instead to create the
        // reponse
        writer.send_can_message(
            can::ids::NodeId::host,
            GripperInfoResponse{
                .message_index = m.message_index,
                .model = 0x0001,
                .serial = eeprom::serial_number::SerialDataCodeType {
                    '2',
                    '0',
                    '2',
                    '2',
                    '1',
                    '1',
                    '1',
                    '5',
                    'A',
                    '0',
                    '1'
                }});
#else
        serial_number_accessor.start_read(m.message_index);
#endif
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

    static auto get_gripper_model(
        const eeprom::serial_number::SerialNumberType &serial) -> uint16_t {
        uint16_t model = 0;
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        const auto *iter = serial.begin() + GRIPPER_MODEL_FIELD_START;
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        iter = bit_utils::bytes_to_int(iter, iter + GRIPPER_MODEL_FIELD_LEN,
                                       model);
        return model;
    }

    static auto get_gripper_data_code(
        const eeprom::serial_number::SerialNumberType &serial)
        -> eeprom::serial_number::SerialDataCodeType {
        eeprom::serial_number::SerialDataCodeType dc;
        std::copy_n(serial.begin() + GRIPPER_DATACODE_START,
                    GRIPPER_DATACODE_LEN, dc.begin());
        return dc;
    }
};
};  // namespace gripper_info
