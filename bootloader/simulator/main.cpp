#include <cstring>

#include "FreeRTOS.h"
#include "bootloader/core/message_handler.h"
#include "can/simlib/sim_canbus.hpp"
#include "common/core/logging.hpp"
#include "task.h"
#include "bootloader/core/version.h"
#include "bootloader/core/node_id.h"


/** The simulator's bootloader */
CANNodeId get_node_id(void) {
    return can_nodeid_pipette_bootloader;
}

/** The simulator's version */
uint32_t get_version(void) {
    return 0xDEADBEEF;
}

/**
 * The CAN bus.
 */
static auto canbus = sim_canbus::SimCANBus(can_transport::create());

/**
 * Handle a new can message
 * @param cb_data callback data
 * @param identifier arbitration id
 * @param data data
 * @param length length of data
 */
void on_can_message(void* cb_data, uint32_t identifier, uint8_t* data,
                    uint8_t length) {
    Message message;
    Message response;

    message.arbitration_id.id = identifier;
    message.size = length;
    std::memcpy(message.data, data,
                std::min(static_cast<std::size_t>(length), sizeof(message.data)));

    auto handle_message_return = handle_message(&message, &response);
    switch (handle_message_return) {
        case handle_message_ok:
            LOG("Message ok. No response\n");
            break;
        case handle_message_has_response:
            LOG("Message ok. Has response\n");
            canbus.send(response.arbitration_id.id, response.data,
                        static_cast<CanFDMessageLength>(response.size));
            break;
        case handle_message_error:
            LOG("Message error.\n");
            break;
        default:
            LOG("Unknown return.\n");
            break;
    }
}

int main() {
    canbus.set_incoming_message_callback(nullptr, on_can_message);

    vTaskStartScheduler();
}
