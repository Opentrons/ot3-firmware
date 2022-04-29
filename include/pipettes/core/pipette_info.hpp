/*
** Functions and definitions for deciding what kind of pipette this is.
*/
#pragma once
#include <array>
#include <cstdint>

#include "can/core/can_writer_task.hpp"
#include "can/core/ids.hpp"
#include "can/core/messages.hpp"
#include "motor-control/core/motor_hardware_interface.hpp"

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
 */
template <message_writer_task::TaskClient CanClient>
class PipetteInfoMessageHandler {
  public:
    /**
     * Constructor
     *
     * @param writer A message writer for sending the response
     */
    explicit PipetteInfoMessageHandler(CanClient &writer)
        : PipetteInfoMessageHandler(
              writer, PipetteInfo{.name = get_name(),
                                  .model = get_model(),
                                  .serial = std::array{'2', '0', '2', '2', '0',
                                                       '3', '2', '1', 'A', '0',
                                                       '5', '\0'}}) {}
    PipetteInfoMessageHandler(CanClient &writer,
                              const PipetteInfo &pipette_info)
        : writer(writer),
          response{.name = static_cast<uint16_t>(pipette_info.name),
                   .model = pipette_info.model} {
        std::copy_n(
            pipette_info.serial.cbegin(),
            std::min(pipette_info.serial.size(), response.serial.size()),
            response.serial.begin());
    }
    PipetteInfoMessageHandler(const PipetteInfoMessageHandler &) = delete;
    PipetteInfoMessageHandler(const PipetteInfoMessageHandler &&) = delete;
    auto operator=(const PipetteInfoMessageHandler &)
        -> PipetteInfoMessageHandler & = delete;
    auto operator=(const PipetteInfoMessageHandler &&)
        -> PipetteInfoMessageHandler && = delete;
    ~PipetteInfoMessageHandler() = default;

    using MessageType = std::variant<std::monostate, PipetteInfoRequest>;

    /**
     * Message handler
     * @param m The incoming message.
     */
    void handle(MessageType &m) {
        std::visit([this](auto o) { this->visit(o); }, m);
    }

  private:
    void visit(std::monostate &) {}

    void visit(PipetteInfoRequest &) {
        writer.send_can_message(can_ids::NodeId::host, response);
    }

    CanClient &writer;
    can_messages::PipetteInfoResponse response;
};
};  // namespace pipette_info
