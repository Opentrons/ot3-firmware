#include <iostream>
#include <variant>

#include "FreeRTOS.h"
#include "can/core/can_buffer_task.hpp"
#include "can/core/dispatch.hpp"
#include "can/core/freertos_can_dispatch.hpp"
#include "can/core/messages.hpp"
#include "common/core/bit_utils.hpp"
#include "common/core/freertos_message_buffer.hpp"
#include "common/core/freertos_task.hpp"
#include "queue.h"

using namespace freertos_message_buffer;
using namespace can_dispatch;
using namespace can_message_buffer;
using namespace can_messages;
using namespace freertos_can_dispatch;
using namespace freertos_task;

struct HandlerA {
    void handle(std::variant<std::monostate, GetSpeedRequest> &m) {
        std::visit([this](auto o) { this->visit(o); }, m);
    }

    void visit(std::monostate &m) {
        std::cout << "HandlerA monostate" << std::endl;
    }

    void visit(GetSpeedRequest &m) {
        std::cout << "HandlerA set speed" << std::endl;
    }
};

struct HandlerB {
    void handle(std::variant<std::monostate, StopRequest> &m) {
        std::visit([this](auto o) { this->visit(o); }, m);
    }

    void visit(std::monostate &m) {
        std::cout << "HandlerB monostate" << std::endl;
    }

    void visit(StopRequest &m) {
        std::cout << "HandlerB heartbeat" << std::endl;
    }
};

using BufferType = FreeRTOMessageBuffer<256>;

// Separate message buffers
static auto a_buffer = FreeRTOMessageBuffer<256>{};
static auto b_buffer = FreeRTOMessageBuffer<256>{};

// Handlers for the parsed messages
static auto a_handler = HandlerA{};
static auto b_handler = HandlerB{};

// Parser listeners
static auto a_builder =
    DispatchParseTarget<HandlerA, GetSpeedRequest>(a_handler);
static auto b_builder = DispatchParseTarget<HandlerB, StopRequest>(b_handler);

// Buffer poller functions
static auto poller_a =
    freertos_can_dispatch::FreeRTOSCanBufferPoller{a_buffer, a_builder};
static auto poller_b =
    freertos_can_dispatch::FreeRTOSCanBufferPoller{b_buffer, b_builder};

// Buffer poller tasks
static auto dispatcher_task_a =
    FreeRTOSTask<256, 5, decltype(poller_a)>("dispatcher_a", poller_a);
static auto dispatcher_task_b =
    FreeRTOSTask<256, 5, decltype(poller_b)>("dispatcher_b", poller_b);

static auto a_target =
    DispatchBufferTarget<BufferType, GetSpeedRequest>{a_buffer};
static auto b_target = DispatchBufferTarget<BufferType, StopRequest>{b_buffer};

static auto dispatcher = Dispatcher(&a_target, &b_target);

static auto buffer = FreeRTOMessageBuffer<1024>{};
static auto poller =
    freertos_can_dispatch::FreeRTOSCanBufferPoller(buffer, dispatcher);
static auto dispatcher_task =
    FreeRTOSTask<256, 5, decltype(poller)>("dispatcher", poller);

void vATaskFunction(void *pvParameters) {
    int i = 0;
    auto buff = std::array<uint8_t, 4>{};
    for (;;) {
        vTaskDelay(1000);
        auto a = bit_utils::int_to_bytes(i, buff.begin(), buff.end());
        buffer.send(buff.data(), buff.size(), 0);
        ++i;
    }
    vTaskDelete(NULL);
}

int main() {
    xTaskCreate(vATaskFunction, "Name", 100, nullptr, 0, nullptr);
    vTaskStartScheduler();
    return 0;
}