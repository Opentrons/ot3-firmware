#include "can/firmware/hal_can_bus.hpp"

#include "common/firmware/gpio.hpp"

using namespace can::hal::bus;

void HalCanBus::hal_state_callback(void* data, hal_can_state_t state) {
    if (data == nullptr) {
        return;
    }
    auto* slf = static_cast<HalCanBus*>(data);

    if (slf->indicator.port == nullptr) {
        return;
    }
    switch (state) {
        case HAL_CAN_STATE_ERROR_PASSIVE:
            slf->indicator_timer.update_period_from_isr(
                BLINK_HALF_PERIOD_ERROR_PASSIVE_MS);
            break;
        case HAL_CAN_STATE_ERROR_ACTIVE:
            slf->indicator_timer.update_period_from_isr(
                BLINK_HALF_PERIOD_ERROR_ACTIVE_MS);
            break;
        case HAL_CAN_STATE_BUS_OFF:
            slf->indicator_timer.stop_from_isr();
            gpio::reset(slf->indicator);
            break;
    }
}

void HalCanBus::handle_timer() {
    if (indicator.port != nullptr) {
        if (gpio::is_set(indicator)) {
            gpio::reset(indicator);
        } else {
            gpio::set(indicator);
        }
    }
}

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
