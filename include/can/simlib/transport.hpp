#pragma once

#include <memory>

namespace can::sim::transport {

/**
 * Abstract base class for communicating with a can bus.
 */
class BusTransportBase {
  public:
    /**
     * Open connection.
     * @return True on success.
     */
    virtual auto open() -> bool = 0;

    /**
     * Close connection.
     */
    virtual void close() = 0;

    /**
     * Write a message
     * @param arb_id  The arbitration id
     * @param buff The message data buffer
     * @param buff_len The length of the buffer
     * @return true on success
     */
    virtual auto write(uint32_t arb_id, const uint8_t* buff, uint32_t buff_len)
        -> bool = 0;

    /**
     * Read a massage
     * @param arb_id will contain the received arbitration id
     * @param buff pointer to the buffer in which to write read data.
     * @param buff_len on input, the length of buffer. on output, the actual
     * number of bytes written to buff.
     * @return true on success.
     */
    virtual auto read(uint32_t& arb_id, uint8_t* buff, uint32_t& buff_len)
        -> bool = 0;

    virtual ~BusTransportBase() = default;
};

/**
 * Create an appropriate BusTransportBase.
 *
 * @return pointer to BusTransportBase.
 */
auto create() -> std::shared_ptr<BusTransportBase>;

}  // namespace can_transport
