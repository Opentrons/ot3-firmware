#pragma once

#include <variant>

#include "hepa-uv/core/hepa_task.hpp"
#include "hepa-uv/core/uv_task.hpp"

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

namespace uv {
namespace message_handler {

using MessageType =
    std::variant<std::monostate, can::messages::SetHepaUVStateRequest,
                 can::messages::GetHepaUVStateRequest>;

template <hepa_task::TaskClient UVTaskClient>
class UVHandler {
  public:
    explicit UVHandler(UVTaskClient &uv_client) : uv_client(uv_client) {}
    UVHandler(const UVHandler &) = delete;
    UVHandler(const UVHandler &&) = delete;
    auto operator=(const UVHandler &) -> UVHandler & = delete;
    auto operator=(const UVHandler &&) -> UVHandler && = delete;
    ~UVHandler() = default;

    void handle(MessageType &can_message) {
        std::visit(
            [this](auto &m) -> void { this->uv_client.send_uv_message(m); },
            can_message);
    }

  private:
    UVTaskClient &uv_client;
};

}  // namespace message_handler
}  // namespace uv