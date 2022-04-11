#include <variant>

#include "spi/core/messages.hpp"

namespace spi {

namespace writer {
using TaskMessage = std::variant<std::monostate, messages::Transact>;

template <template <class> class QueueImpl>
requires MessageQueue<QueueImpl<TaskMessage>, TaskMessage>
class Writer {
  using QueueType = QueueImpl<TaskMessage>;

  public:
    Writer() = default;
    ~Writer() = default;
    Writer(const Writer&) = delete;
    auto operator=(const Writer&) -> Writer& = delete;
    Writer(Writer&&) = delete;
    auto operator=(Writer&&) -> Writer& = delete;

    template <messages::SpiResponseQueue RQType>
    void read(uint16_t device_address, std::size_t read_bytes,
              RQType& response_queue, uint32_t id = 0) {
        messages::Transact message{
            .transaction = {.address = device_address,
                .bytes_to_read =
                std::min(read_bytes, messages::MAX_BUFFER_SIZE),
                .bytes_to_write = 0,
                .write_buffer{}},
            .id = {.token = id, .is_completed_poll = 0},
            .response_writer = messages::ResponseWriter(response_queue)};
        queue->try_write(message);
    }
};

template <template <class> class QueueImpl>
requires MessageQueue<QueueImpl<TaskMessage>, TaskMessage>
struct SpiTransactManager {
  public:
    SpiTransactManager() = default;
    ~SpiTransactManager() = default;
    SpiTransactManager(const SpiTransactManager&) = delete;
    auto operator=(const SpiTransactManager&) -> SpiTransactManager& = delete;
    SpiTransactManager(SpiTransactManager&&) = delete;
    auto operator=(SpiTransactManager&&) -> SpiTransactManager& = delete;

    void set_queue(QueueType* q) { queue = q; }

    auto write(Registers addr, uint32_t command_data, spi::SpiDeviceBase::BufferType rxBuffer) -> void {
        auto txBuffer =
            build_message(addr, spi::SpiDeviceBase::Mode::WRITE, command_data);
        queue->write(SpiTransact{
            txBuffer,
            rxBuffer,
            false
        })
    }

    auto read(Registers addr, uint32_t command_data, spi::SpiDeviceBase::BufferType rxBuffer) {
        // send two transact commands. One should be sending
        auto txBuffer =
            build_message(addr, spi::SpiDeviceBase::Mode::READ, command_data);
        queue->write(SpiTransact{
            txBuffer,
            rxBuffer,
            false
        })
        // second value should be sent to CAN.
        queue->write(SpiTransact{
            txBuffer,
            rxBuffer,
            true
        })
    }

    auto get_lock() -> bool {
        return _mutex.aquire();
    }

    auto release_lock() -> bool {
        return _mutex.release();
    }

  private:
    freertos_synchronization::FreeRTOSMutex _mutex{};

    /**
     * @brief Build a message to send over SPI
     * @param[in] addr The address to write to
     * @param[in] mode The mode to use, either WRITE or READ
     * @param[in] val The contents to write to the address (0 if this is a read)
     * @return An array with the contents of the message, or nothing if
     * there was an error
     */
    static auto build_message(uint8_t addr, spi::SpiDeviceBase::Mode mode,
                              uint32_t val) -> MessageT {
        MessageT buffer = {0};
        auto* iter = buffer.begin();
        auto addr_byte = addr;
        addr_byte |= static_cast<uint8_t>(mode);
        iter = bit_utils::int_to_bytes(addr_byte, iter, buffer.end());
        iter = bit_utils::int_to_bytes(val, iter, buffer.end());
        if (iter != buffer.end()) {
            return MessageT();
        }
        return buffer;
    }
};

} // namespace writer

} // namespace spi