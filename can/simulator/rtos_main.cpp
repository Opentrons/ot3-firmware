#include <variant>

#include "can/core/dispatch.hpp"
#include "can/core/freertos_can_dispatch.hpp"
#include "can/core/message_writer.hpp"
#include "can/core/messages.hpp"
#include "can/simulator/sim_canbus.hpp"
#include "can/simulator/socket_can.hpp"
#include "common/core/freertos_message_buffer.hpp"
#include "common/core/freertos_task.hpp"

using namespace freertos_message_buffer;
using namespace can_dispatch;
using namespace can_message_buffer;
using namespace can_messages;
using namespace freertos_can_dispatch;
using namespace freertos_task;

static auto buffer = FreeRTOMessageBuffer<1024>{};
static auto transport = socket_can::SocketCanTransport{};
static auto canbus = sim_canbus::SimCANBus(transport, buffer);

template <CanBus Bus>
struct Loopback {
    Loopback(Bus &can_bus) : writer{canbus} {}
    Loopback(const Loopback &) = delete;
    Loopback(const Loopback &&) = delete;
    auto operator=(const Loopback &) -> Loopback & = delete;
    auto operator=(const Loopback &&) -> Loopback && = delete;

    void handle(
        std::variant<std::monostate, SetSpeedRequest, GetSpeedRequest> &m) {
        std::visit([this](auto o) { this->visit(o); }, m);
    }

    void visit(std::monostate &m) {}

    template <message_core::Serializable Serializable>
    void visit(const Serializable &m) {
        writer.write(NodeId::host, m);
    }

    can_message_writer::MessageWriter<decltype(canbus)> writer;
};

static auto handler = Loopback{canbus};

static auto dispatcher = can_dispatch::DispatchParseTarget<
    Loopback<decltype(canbus)>, SetSpeedRequest, GetSpeedRequest>{handler};

static auto poller =
    freertos_can_dispatch::FreeRTOSCanBufferPoller(buffer, dispatcher);
static auto dispatcher_task =
    FreeRTOSTask<256, 5, decltype(poller)>("dispatcher", poller);

void can_bus_poll_task(void *) {
    transport.open("vcan0");
    while (canbus.read_message()) {
    }
}

int main() {
    xTaskCreate(can_bus_poll_task, "can_poll", 2048, nullptr, 1, nullptr);
    vTaskStartScheduler();
    return 0;
}