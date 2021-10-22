#pragma once

#include <array>

#include "can/core/can_bus.hpp"
#include "can/core/can_message_buffer.hpp"
#include "can/core/message_core.hpp"
#include "common/core/message_buffer.hpp"

namespace sim_canbus {

using namespace can_bus;

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
template <BusTransport Transport, message_buffer::MessageBuffer Buffer>
requires(!std::movable<Buffer> && !std::copyable<Buffer> &&
         !std::movable<Transport> && !std::copyable<Transport>) class SimCANBus
    : public CanBusWriter {
  public:
    explicit SimCANBus(Transport& transport, Buffer& buffer)
        : transport{transport}, reader{transport, buffer} {}
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
     * Read a single message.
     */
    auto read_message() -> bool { return reader(); }

  private:
    struct Reader {
        Reader(Transport& transport, Buffer& buffer)
            : transport{transport}, writer{buffer} {}

        bool operator()() {
            uint32_t read_length = message_core::MaxMessageSize;
            uint32_t arb_id;

            if (!transport.read(arb_id, read_buffer.data(), read_length)) {
                return false;
            }
            writer.send(arb_id, read_buffer.begin(),
                        read_buffer.begin() + read_length, 0);
            return true;
        }

        Transport& transport;
        can_message_buffer::CanMessageBufferWriter<Buffer> writer;
        std::array<uint8_t, message_core::MaxMessageSize> read_buffer{};
    };

    Transport& transport;
    Reader reader;
};

}  // namespace sim_canbus
