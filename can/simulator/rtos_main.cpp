#include <iostream>
#include <variant>

#include "FreeRTOS.h"
#include "can/core/can_message_buffer.hpp"
#include "can/core/dispatch.hpp"
#include "can/core/freertos_can_dispatch.hpp"
#include "can/core/messages.hpp"
#include "common/core/bit_utils.hpp"
#include "common/core/freertos_message_buffer.hpp"
#include "common/core/freertos_task.hpp"

using namespace freertos_message_buffer;
using namespace can_dispatch;
using namespace can_message_buffer;
using namespace can_messages;
using namespace freertos_can_dispatch;
using namespace freertos_task;

struct HandlerA {
    HandlerA() {}
    HandlerA(const HandlerA &) = delete;
    HandlerA(const HandlerA &&) = delete;
    HandlerA &operator=(const HandlerA &) = delete;
    HandlerA &&operator=(const HandlerA &&) = delete;

    void handle(std::variant<std::monostate, GetSpeedRequest> &m) {
        std::visit([this](auto o) { this->visit(o); }, m);
    }

    void visit(std::monostate &m) {
        std::cout << "HandlerA monostate" << std::endl;
    }

    void visit(GetSpeedRequest &m) {
        std::cout << "HandlerA get speed request" << std::endl;
    }
};

struct HandlerB {
    HandlerB() {}
    HandlerB(const HandlerB &) = delete;
    HandlerB(const HandlerB &&) = delete;
    HandlerB &operator=(const HandlerB &) = delete;
    HandlerB &&operator=(const HandlerB &&) = delete;

    void handle(std::variant<std::monostate, StopRequest> &m) {
        std::visit([this](auto o) { this->visit(o); }, m);
    }

    void visit(std::monostate &m) {
        std::cout << "HandlerB monostate" << std::endl;
    }

    void visit(StopRequest &m) { std::cout << "HandlerB stop" << std::endl; }
};

// Handlers for the parsed messages
static auto a_handler = HandlerA{};
static auto b_handler = HandlerB{};

// The dispatch targets.
static auto a = freertos_can_dispatch::FreeRTOSCanDispatcherTarget<
    256, HandlerA, GetSpeedRequest>{a_handler};
static auto b =
    freertos_can_dispatch::FreeRTOSCanDispatcherTarget<256, HandlerB,
                                                       StopRequest>{b_handler};

// Buffer poller tasks
static auto dispatcher_task_a =
    FreeRTOSTask<256, 5, decltype(a.poller)>("dispatcher_a", a.poller);
static auto dispatcher_task_b =
    FreeRTOSTask<256, 5, decltype(b.poller)>("dispatcher_b", b.poller);

static auto dispatcher = Dispatcher(&a.buffer_target, &b.buffer_target);

static auto buffer = FreeRTOMessageBuffer<1024>{};
static auto poller =
    freertos_can_dispatch::FreeRTOSCanBufferPoller(buffer, dispatcher);
static auto dispatcher_task =
    FreeRTOSTask<256, 5, decltype(poller)>("dispatcher", poller);

void task_message_buffer_populator(void *pvParameters) {
    auto ids = std::array<MessageId, 3>{StopRequest::id, GetSpeedRequest::id,
                                        MoveRequest::id};
    auto i = 0;
    auto buff = std::array<uint8_t, 4>{};
    for (;;) {
        auto arbitration_id = ArbitrationId{.id = 0};
        arbitration_id.parts.message_id = static_cast<uint16_t>(ids[i]);
        std::cout << "Sending arbitration id " << std::hex << arbitration_id.id
                  << std::endl;
        vTaskDelay(1000);
        auto a = bit_utils::int_to_bytes(arbitration_id.id, buff.begin(),
                                         buff.end());
        buffer.send(buff.begin(), buff.end(), 0);
        i = ++i % ids.size();
    }
    vTaskDelete(NULL);
}

int main() {
    xTaskCreate(task_message_buffer_populator, "Name", 100, nullptr, 0,
                nullptr);
    vTaskStartScheduler();
    return 0;
}