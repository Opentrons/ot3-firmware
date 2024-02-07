#pragma once

#include <variant>

#include "hepa-uv/core/hepa_task.hpp"

namespace hepa {
namespace message_handler {

using MessageType =
    std::variant<std::monostate, can::messages::SetHepaFanStateRequest,
                 can::messages::GetHepaFanStateRequest>;

template <hepa_task::TaskClient HepaTaskClient>
class HepaHandler {
  public:
    explicit HepaHandler(HepaTaskClient &hepa_client)
        : hepa_client(hepa_client) {}
    HepaHandler(const HepaHandler &) = delete;
    HepaHandler(const HepaHandler &&) = delete;
    auto operator=(const HepaHandler &) -> HepaHandler & = delete;
    auto operator=(const HepaHandler &&) -> HepaHandler && = delete;
    ~HepaHandler() = default;

    void handle(MessageType &can_message) {
        std::visit(
            [this](auto &m) -> void { this->hepa_client.send_hepa_message(m); },
            can_message);
    }

  private:
    HepaTaskClient &hepa_client;
};

}  // namespace message_handler
}  // namespace hepa