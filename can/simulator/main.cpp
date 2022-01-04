#include <cstdlib>
#include <variant>

#include "can/core/dispatch.hpp"
#include "can/core/freertos_can_dispatch.hpp"
#include "can/core/ids.hpp"
#include "can/core/message_writer.hpp"
#include "can/core/messages.hpp"
#include "can/simlib/sim_canbus.hpp"
#include "can/simlib/transport.hpp"
#include "common/core/freertos_message_buffer.hpp"
#include "common/core/freertos_message_queue.hpp"
#include "common/core/freertos_synchronization.hpp"
#include "common/core/freertos_task.hpp"

using namespace freertos_message_buffer;
using namespace can_dispatch;
using namespace can_message_buffer;
using namespace can_messages;
using namespace freertos_can_dispatch;
using namespace freertos_task;
using namespace freertos_synchronization;

static auto canbus = sim_canbus::SimCANBus(can_transport::create());

/**
 * The parsed message handler. It will be passed into a DispatchParseTarget.
 */
struct Loopback {
    /**
     * Constructor
     * @param writer
     */
    Loopback(can_message_writer::MessageWriter &writer) : writer{writer} {}
    Loopback(const Loopback &) = delete;
    Loopback(const Loopback &&) = delete;
    auto operator=(const Loopback &) -> Loopback & = delete;
    auto operator=(const Loopback &&) -> Loopback && = delete;

    /**
     * The message handling function.
     * @param m A variant of the various message types.
     */
    void handle(std::variant<std::monostate, AddLinearMoveRequest> &m) {
        std::visit([this](auto o) { this->visit(o); }, m);
    }

    void visit(std::monostate &m) {}

    void visit(AddLinearMoveRequest &m) {
        auto response = MoveCompleted{
            .group_id = 0, .seq_id = 1, .current_position = 2, .ack_id = 3};
        writer.send_can_message(NodeId::host, response);
    }

    can_message_writer::MessageWriter &writer;
};

// Message writer.
static auto message_writer =
    can_message_writer::MessageWriter{can_ids::NodeId::host};

// Create global handler
static auto handler = Loopback{message_writer};

// Create a DispatchParseTarget to parse messages and pass them to the handler.
static auto dispatcher =
    can_dispatch::DispatchParseTarget<Loopback, AddLinearMoveRequest>{handler};

// Message buffer for read messages.
static constexpr auto buffer_size = 1024;
static auto read_can_message_buffer = FreeRTOSMessageBuffer<buffer_size>{};

// Reader and writer of message buffer
static auto read_can_message_buffer_control =
    freertos_can_dispatch::FreeRTOSCanBufferControl<buffer_size,
                                                    decltype(dispatcher)>{
        dispatcher};

// Message queue for outgoing messages
static auto can_sender_queue = freertos_message_queue::FreeRTOSMessageQueue<
    message_writer_task::TaskMessage>{};

// Entry point to task that writes messages to canbus
static auto can_bus_write_task_entry = message_writer_task::MessageWriterTask<
    freertos_message_queue::FreeRTOSMessageQueue>{can_sender_queue};

/**
 * The can bus poller.
 */
static auto can_bus_poller =
    FreeRTOSCanReader<buffer_size, decltype(dispatcher)>{
        read_can_message_buffer_control};

/**
 * The message buffer polling task.
 */
static auto can_bus_poll_task =
    FreeRTOSTask<2048, decltype(can_bus_poller), sim_canbus::SimCANBus>{
        can_bus_poller};

/**
 * The message writer task.
 */
static auto can_bus_write_task =
    FreeRTOSTask<2048, decltype(can_bus_write_task_entry),
                 sim_canbus::SimCANBus>{can_bus_write_task_entry};

int main() {
    message_writer.set_queue(&can_sender_queue);

    can_bus_write_task.start(5, "can_write", &canbus);

    can_bus_poll_task.start(5, "can_poll", &canbus);

    vTaskStartScheduler();
    return 0;
}