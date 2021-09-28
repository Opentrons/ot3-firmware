#pragma once

#include <concepts>
#include <cstdint>

#include "can/core/parse.hpp"

namespace can_bus {

enum class CanFilterConfig {
    reject,
    to_fifo0,
    to_fifo1,
    to_fifo0_high_priority,
    to_fifo1_high_priority,
};

enum class CanFilterType {
    /* if the arbitration id matches either of the two values. */
    exact,
    /* if arbitration id is within the range of first and second arguments. */
    range,
    /* if first argument equals the arbitration id masked by second argument. */
    mask,
};

/**
 * Can FD message length.
 */
enum class CanFDMessageLength {
    l0 = 0,
    l1 = 1,
    l2 = 2,
    l3 = 3,
    l4 = 4,
    l5 = 5,
    l6 = 6,
    l7 = 7,
    l8 = 8,
    l12 = 12,
    l16 = 16,
    l20 = 20,
    l24 = 24,
    l32 = 32,
    l48 = 48,
    l64 = 64,
};

/**
 * Round length to the nearest CanFDMessageLength
 * @param length a value between 0-64.
 * @return rounded length
 */
auto to_canfd_length(uint32_t length) -> CanFDMessageLength;

/**
 * Can bus writer abstract base class.
 */
class CanBusWriter {
  public:
    /**
     * Write method.
     *
     * @param arbitration_id A 32-bit arbitration id
     * @param buffer The data buffer.
     * @param buffer_length The buffer length.
     */
    virtual void send(uint32_t arbitration_id, uint8_t* buffer,
                      CanFDMessageLength buffer_length) = 0;

    virtual ~CanBusWriter() {}
};

/**
 * Can incoming message filter setup.
 */
class CanBusFilters {
  public:
    /**
     * Create a filter for incoming messages.
     *
     * @param type the type of filter
     * @param config the filter configuration
     * @param val1 depends on the type. Is either a filter, exact arbitration
     * id, or minimum arbitration id
     * @param val2 depends on the type. Is either a mask, exact arbitration id,
     * or maximum arbitration id
     */
    virtual void add_filter(CanFilterType type, CanFilterConfig config,
                            uint32_t val1, uint32_t val2) = 0;

    virtual ~CanBusFilters() {}
};

}  // namespace can_bus