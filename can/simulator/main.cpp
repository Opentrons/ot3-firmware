#include <cstdlib>
#include <variant>

#include "can/core/dispatch.hpp"
#include "can/core/freertos_can_dispatch.hpp"
#include "can/core/ids.hpp"
#include "can/core/message_writer.hpp"
#include "can/core/messages.hpp"
#include "can/simlib/sim_canbus.hpp"
#include "can/simlib/socket_can.hpp"
#include "common/core/freertos_message_buffer.hpp"
#include "common/core/freertos_synchronization.hpp"
#include "common/core/freertos_task.hpp"

using namespace freertos_message_buffer;
using namespace can_dispatch;
using namespace can_message_buffer;
using namespace can_messages;
using namespace freertos_can_dispatch;
using namespace freertos_task;
using namespace freertos_synchronization;

static auto constexpr ChannelEnvironmentVariableName = "CAN_CHANNEL";
static auto constexpr DefaultChannel = "vcan0";

static auto transport = socket_can::SocketCanTransport<
    freertos_synchronization::FreeRTOSCriticalSection>{};

static auto canbus = sim_canbus::SimCANBus(transport);

/**
 * The parsed message handler. It will be passed into a DispatchParseTarget
 * templetized with the same message types.
 * @tparam Bus A CanBusWriter instance
 */
struct Loopback {
    /**
     * Constructor
     * @param can_bus A CanBus instance.
     */
    Loopback(can_bus::CanBus &can_bus, can_ids::NodeId node_id)
        : writer{canbus, node_id} {}
    Loopback(const Loopback &) = delete;
    Loopback(const Loopback &&) = delete;
    auto operator=(const Loopback &) -> Loopback & = delete;
    auto operator=(const Loopback &&) -> Loopback && = delete;

    /**
     * The message handling function.
     * @param m A variant of the various message types.
     */
    void handle(std::variant<std::monostate, MoveRequest> &m) {
        std::visit([this](auto o) { this->visit(o); }, m);
    }

    void visit(std::monostate &m) {}

    void visit(MoveRequest &m) {
        auto response = MoveCompleted{
            .group_id = 0, .seq_id = 1, .current_position = 2, .ack_id = 3};
        writer.write(NodeId::host, response);
    }

    can_message_writer::MessageWriter writer;
};

// Create global handler
static auto handler = Loopback{canbus, can_ids::NodeId::host};

// Create a DispatchParseTarget to parse messages in message buffer and pass
// them to the handler.
static auto dispatcher =
    can_dispatch::DispatchParseTarget<Loopback, MoveRequest>{handler};

// Message buffer for read messages.
static auto read_can_message_buffer = FreeRTOSMessageBuffer<1024>{};
static auto read_can_message_buffer_writer =
    can_message_buffer::CanMessageBufferWriter(read_can_message_buffer);

/**
 * New CAN message callback.
 *
 * @param identifier Arbitration id
 * @param data Message data
 * @param length Message data length
 */
void callback(uint32_t identifier, uint8_t *data, uint8_t length) {
    read_can_message_buffer_writer.send_from_isr(identifier, data,
                                                 data + length);
}

/**
 * The socket can reader. Reads from socket and writes to message buffer.
 */
void can_bus_poll_task_entry() {
    const char *env_channel_val = std::getenv(ChannelEnvironmentVariableName);
    auto channel = env_channel_val ? env_channel_val : DefaultChannel;

    transport.open(channel);

    // A Message Buffer poller that reads from buffer and send to dispatcher
    static auto poller = freertos_can_dispatch::FreeRTOSCanBufferPoller(
        read_can_message_buffer, dispatcher);
    poller();
}

/**
 * The message buffer polling task.
 */
static auto can_bus_poll_task =
    FreeRTOSTask<2048, 5>("can_poll", can_bus_poll_task_entry);

int main() {
    vTaskStartScheduler();
    return 0;
}