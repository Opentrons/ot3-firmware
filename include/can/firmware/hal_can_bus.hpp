#pragma once

#include "can/core/bit_timings.hpp"
#include "can/core/can_bus.hpp"
#include "can/firmware/hal_can.h"
#include "common/core/freertos_timer.hpp"
#include "common/firmware/gpio.hpp"
namespace can::hal::bus {

using namespace can::bus;
using namespace can::ids;

/**
 * HAL FD CAN wrapper.
 */
class HalCanBus : public CanBus {
  public:
    /**
     * Construct
     * @param handle A pointer to an initialized FDCAN_HandleTypeDef
     */
    explicit HalCanBus(HAL_CAN_HANDLE handle) : HalCanBus(handle, {}) {}
    HalCanBus(HAL_CAN_HANDLE handle, gpio::PinConfig indicator_pin)
        : indicator_timer{"can indicator", [this]() -> void { handle_timer(); },
                          BLINK_HALF_PERIOD_ERROR_ACTIVE_MS},
          handle{handle},
          indicator{indicator_pin} {}

    HalCanBus(const HalCanBus&) = delete;
    auto operator=(const HalCanBus&) -> HalCanBus& = delete;
    HalCanBus(HalCanBus&&) = delete;
    auto operator=(HalCanBus&&) -> HalCanBus& = delete;
    ~HalCanBus() final = default;

    template <can::bit_timings::BitTimingSpec TimingsT>
    void start(TimingsT timings) {
        if (indicator.port != nullptr) {
            indicator_timer.start();
        }
        can_start(timings.clock_divider, timings.segment_1_quanta,
                  timings.segment_2_quanta, timings.max_sync_jump_width);
    }

    /**
     * Set the incoming message callback.
     * @param cb_data data passed back to caller in callback.
     * @param callback message to be called with new CAN message.
     */
    void set_incoming_message_callback(void* cb_data,
                                       IncomingMessageCallback callback) final;

    /**
     * Add an arbitration id filter.
     *
     * @param type the type of filter
     * @param config the filter configuration
     * @param val1 depends on the type. Is either a filter, exact arbitration
     * id, or minimum arbitration id
     * @param val2 depends on the type. Is either a mask, exact arbitration id,
     * or maximum arbitration id
     */
    void add_filter(CanFilterType type, CanFilterConfig config, uint32_t val1,
                    uint32_t val2) final;

    /**
     * Send a buffer on can bus
     * @param arbitration_id The arbitration id
     * @param buffer buffer to send
     * @param buffer_length length of buffer
     */
    void send(uint32_t arbitration_id, uint8_t* buffer,
              CanFDMessageLength buffer_length) final;

  private:
    static constexpr uint32_t BLINK_HALF_PERIOD_ERROR_PASSIVE_MS = 50;
    static constexpr uint32_t BLINK_HALF_PERIOD_ERROR_ACTIVE_MS = 250;
    void handle_timer();
    static void hal_state_callback(void* data, hal_can_state_t state);
    freertos_timer::FreeRTOSTimer indicator_timer;
    HAL_CAN_HANDLE handle;
    uint32_t filter_index = 0;
    gpio::PinConfig indicator;
};

}  // namespace hal_can_bus
