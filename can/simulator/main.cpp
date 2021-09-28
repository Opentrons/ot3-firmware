#include <cstdlib>
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

static auto constexpr ChannelEnvironmentVariableName = "CAN_CHANNEL";
static auto constexpr DefaultChannel = "vcan0";

static auto constexpr ReceiveBufferSize = 1024;
static auto buffer = FreeRTOSMessageBuffer<ReceiveBufferSize>{};
static auto transport = socket_can::SocketCanTransport{};
static auto canbus = sim_canbus::SimCANBus(transport, buffer);

/**
 * The parsed message handler. It will be passed into a DispatchParseTarget
 * templetized with the same message types.
 * @tparam Bus A CanBus type
 */
struct Loopback {
    /**
     * Constructor
     * @param can_bus A CanBus instance.
     */
    Loopback(CanBusWriter &can_bus) : writer{canbus} {}
    Loopback(const Loopback &) = delete;
    Loopback(const Loopback &&) = delete;
    auto operator=(const Loopback &) -> Loopback & = delete;
    auto operator=(const Loopback &&) -> Loopback && = delete;

    /**
     * The message handling function.
     * @param m A variant of the various message types.
     */
    void handle(std::variant<std::monostate, MoveRequest, GetSpeedRequest> &m) {
        std::visit([this](auto o) { this->visit(o); }, m);
    }

    void visit(std::monostate &m) {}

    /**
     * Handler for all message types.
     * @param m Serializable.
     */
    template <message_core::Serializable Serializable>
    void visit(const Serializable &m) {
        // Loop it back
        writer.write(NodeId::host, m);
    }

    can_message_writer::MessageWriter writer;
};

// Create global handler
static auto handler = Loopback{canbus};

// Create a DispatchParseTarget to parse messages in message buffer and pass
// them to the handler.
static auto dispatcher =
    can_dispatch::DispatchParseTarget<Loopback, MoveRequest, GetSpeedRequest>{
        handler};

// A Message Buffer poller that reads from buffer and send to dispatcher
static auto poller =
    freertos_can_dispatch::FreeRTOSCanBufferPoller(buffer, dispatcher);
// The message buffer freertos task
static auto dispatcher_task =
    FreeRTOSTask<256, 5, decltype(poller)>("dispatcher", poller);

/**
 * The socket can reader. Reads from socket and writes to message buffer.
 */
void can_bus_poll_task(void *) {
    const char *env_channel_val = std::getenv(ChannelEnvironmentVariableName);
    auto channel = env_channel_val ? env_channel_val : DefaultChannel;

    transport.open(channel);
    while (canbus.read_message()) {
    }
}

int main() {
    xTaskCreate(can_bus_poll_task, "can_poll", 2048, nullptr, 1, nullptr);
    vTaskStartScheduler();
    return 0;
}