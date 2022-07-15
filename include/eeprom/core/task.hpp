#pragma once

#include "common/core/bit_utils.hpp"
#include "common/core/buffer_type.hpp"
#include "common/core/logging.h"
#include "common/core/message_queue.hpp"
#include "common/core/message_utils.hpp"
#include "eeprom/core/messages.hpp"
#include "eeprom/core/types.hpp"
#include "hardware_iface.hpp"
#include "i2c/core/messages.hpp"
#include "i2c/core/transaction.hpp"
#include "i2c/core/writer.hpp"

namespace eeprom {
namespace task {

using TaskMessage =
    std::variant<message::WriteEepromMessage, message::ReadEepromMessage,
                 i2c::messages::TransactionResponse, std::monostate>;

template <class I2CQueueWriter, class OwnQueue>
class EEPromMessageHandler {
  public:
    explicit EEPromMessageHandler(I2CQueueWriter &i2c_writer,
                                  OwnQueue &own_queue,
                                  hardware_iface::EEPromHardwareIface &hw_iface)
        : writer{i2c_writer}, own_queue{own_queue}, hw_iface{hw_iface} {}
    EEPromMessageHandler(const EEPromMessageHandler &) = delete;
    EEPromMessageHandler(const EEPromMessageHandler &&) = delete;
    auto operator=(const EEPromMessageHandler &)
        -> EEPromMessageHandler & = delete;
    auto operator=(const EEPromMessageHandler &&)
        -> EEPromMessageHandler && = delete;
    ~EEPromMessageHandler() = default;

    static constexpr auto WRITE_TOKEN = static_cast<uint32_t>(-1);
    static constexpr auto MAX_INFLIGHT_READS = 10;

    void handle_message(TaskMessage &m) {
        std::visit([this](auto o) { this->visit(o); }, m);
    }

  private:
    void visit(std::monostate &) {}

    /**
     * Handle a transaction response from the i2c task
     * @param m The message
     */
    void visit(i2c::messages::TransactionResponse &m) {
        LOG("Transaction with token %ud has completed.", m.id.token);
        if (m.id.token == WRITE_TOKEN) {
            // A write has completed
            hw_iface.enable();
        } else {
            // A read has completed
            auto id_map_entry = id_map.remove(m.id.token);
            if (id_map_entry) {
                auto read_message = id_map_entry.value();
                auto data = types::EepromData{};
                std::copy_n(m.read_buffer.cbegin(),
                            std::min(m.bytes_read, data.size()), data.begin());
                auto v = message::EepromMessage{
                    .memory_address = read_message.memory_address,
                    .length = static_cast<types::data_length>(m.bytes_read),
                    .data = data};
                read_message.callback(v, read_message.callback_param);
            } else {
                LOG("Failed to find entry for token %ud.", m.id.token);
            }
        }
    }

    /**
     * Handle a request to write to eeprom.
     * @param m THe message
     */
    void visit(message::WriteEepromMessage &m) {
        LOG("Received request to write %d bytes to address %x", m.length,
            m.memory_address);

        if (m.length <= 0) {
            return;
        }

        // On the Microchip variant of the eeprom if you attempt to write
        // that crosses the page boundry (8 Bytes) it will wrap and overwrite
        // the begining of the current page instead of moving to the next page

        if (hw_iface.get_eeprom_chip_type() ==
                hardware_iface::EEPromChipType::MICROCHIP_24AA02T &&
            ((m.memory_address % 8) + m.length) > 8) {
            LOG("Warning: write request will overrun page");
        }

        // The ST eeprom has a page write function but that requires driving
        // the write enable pin differently which would require us to change
        // how we use enable_eeprom_write disable_eeprom_write calls in each
        // firmware implementation however this would allow us to write 64
        // bytes/cycle which may be performance boost.

        auto buffer = i2c::messages::MaxMessageBuffer{};
        auto *iter = buffer.begin();

        // Older boards use 1 byte addresses, if we're on an older board we
        // need to drop the higher byte of the memory address when sending the
        // the message over i2c
        if (hw_iface.get_eeprom_addr_bytes() ==
            static_cast<size_t>(
                hardware_iface::EEPromAddressType::EEPROM_ADDR_8_BIT)) {
            m.memory_address = m.memory_address << 8;
        }
        iter = bit_utils::int_to_bytes(
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            m.memory_address, iter, (iter + hw_iface.get_eeprom_addr_bytes()));
        // Remainder is data
        iter = std::copy_n(
            m.data.cbegin(),
            std::min(buffer.size() - 1, static_cast<std::size_t>(m.length)),
            iter);
        // A write transaction.
        auto transaction = i2c::messages::Transaction{
            .address = types::DEVICE_ADDRESS,
            .bytes_to_read = 0,
            .bytes_to_write = static_cast<std::size_t>(iter - buffer.begin()),
            .write_buffer = buffer};
        // Use the WRITE_TOKEN to disambiguate from the reads.
        auto transaction_id =
            i2c::messages::TransactionIdentifier{.token = WRITE_TOKEN};
        hw_iface.disable();
        if (!writer.transact(transaction, transaction_id, own_queue)) {
            // Failed to write transaction. Re-enable write protection.
            hw_iface.enable();
        }
    }

    /**
     * Handle a request to read from the eeprom
     * @param m The message
     */
    void visit(message::ReadEepromMessage &m) {
        LOG("Received request to read %d bytes from address %d", m.length,
            m.memory_address);

        if (m.length <= 0) {
            return;
        }

        auto token = id_map.add(m);
        if (!token) {
            LOG("No space in the id map.");
            // TODO (amit, 2022-05-04): Should we re-enqueue the message?
            return;
        }

        // The transaction will write the memory address, then read the
        // data.
        auto write_buffer = i2c::messages::MaxMessageBuffer{};
        auto *iter = write_buffer.begin();

        // Older boards use 1 byte addresses, if we're on an older board we
        // need to drop the higher byte of the memory address when sending the
        // the message over i2c
        if (hw_iface.get_eeprom_addr_bytes() ==
            static_cast<size_t>(
                hardware_iface::EEPromAddressType::EEPROM_ADDR_8_BIT)) {
            m.memory_address = m.memory_address << 8;
        }

        iter = bit_utils::int_to_bytes(
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            m.memory_address, iter, (iter + hw_iface.get_eeprom_addr_bytes()));

        auto transaction = i2c::messages::Transaction{
            .address = types::DEVICE_ADDRESS,
            .bytes_to_read = m.length,
            .bytes_to_write =
                static_cast<std::size_t>(iter - write_buffer.begin()),
            .write_buffer{write_buffer}};
        // The transaction identifier uses the token returned from id_map.add
        auto transaction_id =
            i2c::messages::TransactionIdentifier{.token = token.value()};

        if (!writer.transact(transaction, transaction_id, own_queue)) {
            // The writer cannot accept this message. Remove it from the id_map.
            id_map.remove(token.value());
        }
    }

    I2CQueueWriter &writer;
    OwnQueue &own_queue;
    i2c::transaction::IdMap<message::ReadEepromMessage, MAX_INFLIGHT_READS>
        id_map{};
    hardware_iface::EEPromHardwareIface &hw_iface;
};

/**
 * The task type.
 */
template <template <class> class QueueImpl>
requires MessageQueue<QueueImpl<TaskMessage>, TaskMessage>
class EEPromTask {
  public:
    using Messages = TaskMessage;
    using QueueType = QueueImpl<TaskMessage>;
    EEPromTask(QueueType &queue) : queue{queue} {}
    EEPromTask(const EEPromTask &c) = delete;
    EEPromTask(const EEPromTask &&c) = delete;
    auto operator=(const EEPromTask &c) = delete;
    auto operator=(const EEPromTask &&c) = delete;
    ~EEPromTask() = default;

    /**
     * Task entry point.
     */
    [[noreturn]] void operator()(i2c::writer::Writer<QueueImpl> *writer,
                                 hardware_iface::EEPromHardwareIface *pin) {
        auto handler = EEPromMessageHandler{*writer, get_queue(), *pin};
        TaskMessage message{};
        for (;;) {
            if (queue.try_read(&message, queue.max_delay)) {
                handler.handle_message(message);
            }
        }
    }

    [[nodiscard]] auto get_queue() const -> QueueType & { return queue; }

  private:
    QueueType &queue;
};

/**
 * Concept describing a class that can message this task.
 * @tparam Client
 */
template <typename Client>
concept TaskClient = requires(Client client, const TaskMessage &m) {
    {client.send_eeprom_queue(m)};
};

}  // namespace task
}  // namespace eeprom
