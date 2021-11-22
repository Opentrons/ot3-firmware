#pragma once

#include <array>

#include "can/core/can_bus.hpp"
#include "can/core/message_core.hpp"
#include "common/core/freertos_task.hpp"

namespace sim_canbus {

using namespace can_bus;
using namespace freertos_task;

template <class T>
concept BusTransport = requires(T t, uint32_t arb_id, uint32_t& out_arb_id,
                                uint8_t* buff, const uint8_t* cbuff,
                                uint32_t buff_len, uint32_t& in_out_buff_len) {
    { t.write(arb_id, cbuff, buff_len) } -> std::convertible_to<bool>;
    { t.read(out_arb_id, buff, in_out_buff_len) } -> std::convertible_to<bool>;
};

/**
 * CAN bus for simulators. Matches the CanBus concept.
 */
template <BusTransport Transport>
requires(!std::movable<Transport> && !std::copyable<Transport>) class SimCANBus
    : public CanBus {
  public:
    explicit SimCANBus(Transport& transport)
        : transport{transport}, reader{transport}, reader_task{"", reader} {}
    SimCANBus(const SimCANBus&) = delete;
    SimCANBus(const SimCANBus&&) = delete;
    SimCANBus& operator=(const SimCANBus&) = delete;
    SimCANBus&& operator=(const SimCANBus&&) = delete;

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
                    uint32_t val2) {}

    /**
     * Send a buffer on can bus
     * @param arbitration_id The arbitration id
     * @param buffer buffer to send
     * @param buffer_length length of buffer
     */
    void send(uint32_t arbitration_id, uint8_t* buffer,
              CanFDMessageLength buffer_length) {
        transport.write(arbitration_id, buffer,
                        static_cast<uint32_t>(buffer_length));
    }

    /**
     * Set incoming message callback.
     *
     * @param callback
     */
    virtual void set_incoming_message_callback(
        IncomingMessageCallback callback) {
        reader.new_message_callback = callback;
    }

  private:
    struct Reader {
        Reader(Transport& transport) : transport{transport} {}

        void operator()() {
            while (true) {
                uint32_t read_length = message_core::MaxMessageSize;
                uint32_t arb_id;

                if (!transport.read(arb_id, read_buffer.data(), read_length)) {
                    continue;
                }

                if (new_message_callback) {
                    new_message_callback(arb_id, read_buffer.begin(),
                                         read_length);
                }
            }
        }

        Transport& transport;
        std::array<uint8_t, message_core::MaxMessageSize> read_buffer{};
        IncomingMessageCallback new_message_callback{nullptr};
    };

    Transport& transport;
    Reader reader;
    FreeRTOSTask<256, 5> reader_task;
};

}  // namespace sim_canbus
