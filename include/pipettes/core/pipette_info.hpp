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

enum class PipetteName {
    P1000_SINGLE = 0,
    P1000_MULTI = 1,
    P1000_96 = 2,
    P1000_384 = 3,
};

struct PipetteInfo {
    PipetteName name;
    uint16_t model;
    eeprom::serial_number::SerialNumberType serial;
};

// This is currently implemented in pipette-type-specific source files in core
// e.g. pipettes/core/pipette_type_single.cpp but could probably also be parsed
// from the serial number like the model below
PipetteName get_name();

// These defines are used to help parse pipette serials
// A full Serial number looks like P1KSV20201907243 and contains the name, model
// and individual data code [name]P1K [model]SV [data_code] 20201907243
constexpr size_t PIPETTE_NAME_FIELD_START = 0;
constexpr size_t PIPETTE_NAME_FIELD_LEN = 3;
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
    void read_complete() final {
        std::array<uint8_t, eeprom::addresses::serial_number_length> serial{};
        std::copy_n(sn_accessor_backing.begin(),
                    eeprom::addresses::serial_number_length, serial.begin());
        writer.send_can_message(
            can::ids::NodeId::host,
            can::messages::PipetteInfoResponse{
                .name = static_cast<uint16_t>(get_name()),
                .model = get_model(sn_accessor_backing),
                .serial = get_data_code(sn_accessor_backing)});
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
        std::copy_n(m.serial.begin(), sn_accessor_backing.size(),
                    sn_accessor_backing.begin());
        serial_number_accessor.write(sn_accessor_backing);
    }

    CanClient &writer;
    eeprom::serial_number::SerialNumberType sn_accessor_backing =
        eeprom::serial_number::SerialNumberType{};
    eeprom::serial_number::SerialNumberAccessor<EEPromClient>
        serial_number_accessor;
    static auto get_model(const eeprom::serial_number::SerialNumberType &serial)
        -> uint16_t {
        uint16_t model = 0;
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        const auto *iter = serial.begin() + PIPETTE_MODEL_FIELD_START;
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        iter = bit_utils::bytes_to_int(iter, iter + PIPETTE_MODEL_FIELD_LEN,
                                       model);
        return model;
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
