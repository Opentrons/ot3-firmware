#pragma once

#include <array>
#include <memory>
#include <vector>

#include "can/core/can_bus.hpp"
#include "can/core/message_core.hpp"
#include "can/simlib/filter.hpp"
#include "can/simlib/transport.hpp"
#include "common/core/freertos_task.hpp"
#include "common/core/logging.h"

namespace sim_canbus {

using namespace can_bus;
using namespace freertos_task;

/**
 * CAN bus for simulators. Matches the CanBus concept.
 */
class SimCANBus : public CanBus {
  public:
    using TransportType = std::shared_ptr<can_transport::BusTransportBase>;

    explicit SimCANBus(TransportType transport)
        : transport{transport}, reader_task{reader} {
        reader_task.start(5, "", this);
    }
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
                    uint32_t val2) {
        filters.push_back(sim_filter::Filter(type, config, val1, val2));
    }

    /**
     * Send a buffer on can bus
     * @param arbitration_id The arbitration id
     * @param buffer buffer to send
     * @param buffer_length length of buffer
     */
    void send(uint32_t arbitration_id, uint8_t* buffer,
              CanFDMessageLength buffer_length) {
        transport->write(arbitration_id, buffer,
                         static_cast<uint32_t>(buffer_length));
    }

    /**
     * Set incoming message callback.
     *
     * @param cb_data data that will be passed back to the callback.
     * @param callback callback function reporting new CAN message.
     */
    virtual void set_incoming_message_callback(
        void* cb_data, IncomingMessageCallback callback) {
        new_message_callback_data = cb_data;
        new_message_callback = callback;
    }

  private:
    struct Reader {
        void operator()(SimCANBus* bus) {
            while (true) {
                if (!bus->transport->open()) {
                    LOG("Failed to connect.\n");
                    vTaskDelay(2000);
                    continue;
                }

                _read_messages(bus);

                bus->transport->close();
            }
        }

        void _read_messages(SimCANBus* bus) {
            while (true) {
                uint32_t read_length = message_core::MaxMessageSize;
                uint32_t arb_id;

                if (!bus->transport->read(arb_id, read_buffer.data(),
                                          read_length)) {
                    LOG("Disconnected.\n");
                    break;
                }

                // If there are filters and any of them return true we can
                // accept the message.
                if (bus->filters.empty() ||
                    std::any_of(
                        bus->filters.cbegin(), bus->filters.cend(),
                        [arb_id](const auto& f) { return f(arb_id); })) {
                    if (bus->new_message_callback) {
                        bus->new_message_callback(
                            bus->new_message_callback_data, arb_id,
                            read_buffer.begin(), read_length);
                    }
                } else {
                    LOG("Message with arb_id %X and length %d is rejected\n",
                        arb_id, read_length);
                }
            }
        }

        std::array<uint8_t, message_core::MaxMessageSize> read_buffer{};
    };

    TransportType transport;
    Reader reader;
    void* new_message_callback_data{nullptr};
    IncomingMessageCallback new_message_callback{nullptr};
    FreeRTOSTask<256, Reader, SimCANBus> reader_task;
    std::vector<sim_filter::Filter> filters{};
};

}  // namespace sim_canbus
