/*
** Functions and definitions for deciding what kind of gripper this is.
*/
#pragma once
#include <array>
#include <cstdint>

#include "can/core/can_writer_task.hpp"
#include "can/core/ids.hpp"
#include "can/core/messages.hpp"

namespace gripper_info {
using namespace can_ids;
using namespace can_messages;

struct GripperInfo {
    uint16_t model;
    std::array<char, 12> serial;
};

/**
 * A HandlesMessages implementing class that will respond to system messages.
 *
 * @tparam CanClient can writer task client
 */
template <message_writer_task::TaskClient CanClient>
class GripperInfoMessageHandler {
  public:
    /**
     * Constructor
     *
     * @param writer A message writer for sending the response
     */
    explicit GripperInfoMessageHandler(CanClient &writer)
        : GripperInfoMessageHandler(
              writer, GripperInfo{.model = 1,
                                  .serial = std::array{'2', '0', '2', '2', '0',
                                                       '4', '2', '1', 'A', '0',
                                                       '5', '\0'}}) {}
    GripperInfoMessageHandler(CanClient &writer,
                              const GripperInfo &gripper_info)
        : writer(writer), response{.model = gripper_info.model} {
        std::copy_n(
            gripper_info.serial.cbegin(),
            std::min(gripper_info.serial.size(), response.serial.size()),
            response.serial.begin());
    }
    GripperInfoMessageHandler(const GripperInfoMessageHandler &) = delete;
    GripperInfoMessageHandler(const GripperInfoMessageHandler &&) = delete;
    auto operator=(const GripperInfoMessageHandler &)
        -> GripperInfoMessageHandler & = delete;
    auto operator=(const GripperInfoMessageHandler &&)
        -> GripperInfoMessageHandler && = delete;
    ~GripperInfoMessageHandler() = default;

    using MessageType = std::variant<std::monostate, GripperInfoRequest>;

    /**
     * Message handler
     * @param m The incoming message.
     */
    void handle(MessageType &m) {
        std::visit([this](auto o) { this->visit(o); }, m);
    }

  private:
    void visit(std::monostate &m) {}

    void visit(GripperInfoRequest &m) {
        writer.send_can_message(can_ids::NodeId::host, response);
    }

    CanClient &writer;
    can_messages::GripperInfoResponse response;
};
};  // namespace gripper_info
