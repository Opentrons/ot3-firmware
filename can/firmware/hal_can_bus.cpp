#include "can/firmware/hal_can_bus.hpp"

using namespace hal_can_bus;

void HalCanBus::set_incoming_message_callback(
    void* cb_data, IncomingMessageCallback callback) {
    can_register_message_callback(cb_data, callback);
}

void HalCanBus::add_filter(CanFilterType type, CanFilterConfig config,
                           uint32_t val1, uint32_t val2) {
    can_add_filter(handle, filter_index++, type, config, val1, val2);
}

void HalCanBus::send(uint32_t arbitration_id, uint8_t* buffer,
                     CanFDMessageLength buffer_length) {
    can_send_message(handle, arbitration_id, buffer, buffer_length);
}
