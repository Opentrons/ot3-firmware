#include <algorithm>
#include <bitset>
#include <cstdint>
#include <variant>
#include <vector>

extern "C" {
__attribute__((weak)) void crc16_init(void) {}

__attribute__((weak)) uint16_t crc16_compute(const uint8_t* data,
                                             uint8_t length) {
    if (length == 0 || data == nullptr) {
        return 0;
    }

    std::vector<uint8_t> data_vec(data, data + length);
    std::bitset<2048> data_bitset;
    for (size_t i = 0; i < length; ++i) {
        for (size_t bit = 0; bit < 8; ++bit) {
            data_bitset[(i * 8) + bit] = (data_vec[i] >> bit) & 1;
        }
    }

    std::bitset<17> generator(0b10001000000100001);
    constexpr uint16_t generator_position = 16;

    std::bitset<2048 + generator_position> bit_data;
    for (size_t i = 0; i < length * 8; i++) {
        bit_data[i] = data_bitset[i];
    }
    bit_data <<= generator_position;
    uint16_t data_position = (length * 8) + generator_position - 1;

    while (data_position >= generator_position) {
        if (!bit_data.test(data_position)) {
            data_position--;
            continue;
        }
        uint16_t difference = data_position - generator_position;
        std::bitset<2048 + generator_position> divisor(generator.to_ullong());
        divisor <<= difference;
        bit_data ^= divisor;
    }

    std::bitset<16> crc;
    for (int i = 15; i >= 0; i--) {
        crc[i] = bit_data[i];
    }
    return static_cast<uint16_t>(crc.to_ullong());
}
}

#include "catch2/catch.hpp"
#include "common/tests/mock_message_queue.hpp"
#include "eeprom/core/book_accessor.hpp"
#include "eeprom/core/messages.hpp"
#include "eeprom/core/task.hpp"
#include "eeprom/core/types.hpp"
#include "eeprom/firmware/crc16.h"
#include "eeprom/tests/mock_eeprom_listener.hpp"
#include "i2c/core/writer.hpp"
#include "i2c/tests/mock_response_queue.hpp"

using namespace eeprom;

class MockHardwareIface : public hardware_iface::EEPromHardwareIface {
    using hardware_iface::EEPromHardwareIface::EEPromHardwareIface;

  public:
    void set_write_protect(bool enabled) { set_calls.push_back(enabled); }
    std::vector<bool> set_calls{};
};

enum ReadOption {
    VALID,
    ONE_INVALID,
    ALL_INVALID,
};

template <class I2CQueueWriter, class OwnQueue>
struct BMockEEpromTaskClient {
    ReadOption read_option = VALID;

    BMockEEpromTaskClient() {
        writer.set_queue(&i2c_queue);
        backing.fill(0xFF);
    }

    task::EEPromMessageHandler<I2CQueueWriter, OwnQueue> true_eeprom_handler =
        task::EEPromMessageHandler{writer, response_queue, hardware_iface};

    void send_eeprom_queue(const task::TaskMessage& message) {
        std::visit(
            [this](auto o) -> auto { this->visit(o); }, message);
    }

    std::vector<task::TaskMessage> messages_received{};
    std::array<uint8_t, 16384> backing{};

  private:
    test_mocks::MockMessageQueue<i2c::writer::TaskMessage> i2c_queue{};
    test_mocks::MockI2CResponseQueue response_queue{};
    i2c::writer::Writer<test_mocks::MockMessageQueue> writer =
        i2c::writer::Writer<test_mocks::MockMessageQueue>{};
    MockHardwareIface hardware_iface =
        MockHardwareIface{hardware_iface::EEPromChipType::ST_M24128_BF};

    void visit(const message::ReadEepromMessage& message) {
        message::EepromMessage to_be_sent = eeprom::message::EepromMessage{};
        to_be_sent.memory_address = message.memory_address;
        to_be_sent.length = message.length;
        to_be_sent.message_index = message.message_index;

        auto data_to_be_sent =
            std::array<uint8_t, static_cast<size_t>(types::page_length / 2)>{};

        // 1. Read directly from the EEPROM replica
        std::copy_n(&backing[message.memory_address], message.length,
                    data_to_be_sent.begin());
        // 2. Identify if this is the first chunk (where the CRC sits in bytes 0
        // and 1)
        bool is_first_chunk =
            (message.memory_address % types::page_length) == 0;

        // 3. CRITICAL FIX: Ensure we are only corrupting data partition reads,
        // not the lookup table or headers!
        bool is_data_partition =
            message.memory_address >= eeprom::addresses::ot_library_begin &&
            message.memory_address < eeprom::addresses::ot_library_end;

        if (is_first_chunk && is_data_partition) {
            if (read_option == ALL_INVALID) {
                // Change all CRCs to 0
                data_to_be_sent[0] = static_cast<uint8_t>(~data_to_be_sent[0]);
                data_to_be_sent[1] = static_cast<uint8_t>(~data_to_be_sent[1]);
            } else if (read_option == ONE_INVALID) {
                // Determine the start of the book (4 pages per book) RELATIVE
                // to ot_library_begin
                uint16_t offset_from_begin =
                    message.memory_address -
                    eeprom::addresses::ot_library_begin;
                uint16_t book_start =
                    message.memory_address -
                    (offset_from_begin %
                     (types::page_length * types::pages_per_book));

                uint16_t max_counter = 0;
                uint16_t most_recent_addr = book_start;

                // Find the most recent page by iterating through the counters
                for (int i = 0; i < types::pages_per_book; i++) {
                    uint16_t addr = book_start + (i * types::page_length);
                    uint16_t counter;
                    std::memcpy(&counter, &backing[addr + 2], sizeof(counter));

                    // Ignore uninitialized pages (0xFFFF) and find max
                    if (counter != 0xFFFF) {
                        // Handle potential wrap-around when finding max
                        if (max_counter > 65000 && counter < 1000) {
                            max_counter = counter;
                            most_recent_addr = addr;
                        } else if (counter >= max_counter &&
                                   (counter - max_counter) < 1000) {
                            max_counter = counter;
                            most_recent_addr = addr;
                        }
                    }
                }

                // If THIS read is the most recent one, corrupt the CRC to
                // trigger cascade
                if (message.memory_address == most_recent_addr) {
                    data_to_be_sent[0] = ~data_to_be_sent[0];
                }
            }
        }

        to_be_sent.data = data_to_be_sent;
        message.callback(to_be_sent, message.callback_param);
        messages_received.push_back(message);
    }

    void visit(const message::WriteEepromMessage& message) {
        // Apply writes directly to the EEPROM replica
        std::copy_n(message.data.begin(), message.length,
                    &backing[message.memory_address]);
        messages_received.push_back(message);
    }

    void visit(const std::monostate& message) {
        std::ignore = message;
        messages_received.push_back(message);
    }

    void visit(const message::ConfigRequestMessage& message) {
        messages_received.push_back(message);
        auto m = task::TaskMessage{message};
        true_eeprom_handler.handle_message(m);
    }

    void visit(const i2c::messages::TransactionResponse& message) {
        messages_received.push_back(message);
    }
};

// ============================================================================
// HELPER LAMBDA MACRO
// ============================================================================
#define DEFINE_CHECK_WRITE_HELPER                                              \
    auto check_write = [&](uint16_t expected_counter) {                        \
        for (auto it = mock_client.messages_received.rbegin();                 \
             it != mock_client.messages_received.rend(); ++it) {               \
            if (std::holds_alternative<eeprom::message::WriteEepromMessage>(   \
                    *it)) {                                                    \
                auto w = std::get<eeprom::message::WriteEepromMessage>(*it);   \
                if (w.memory_address >= eeprom::addresses::ot_library_begin && \
                    w.memory_address < eeprom::addresses::ot_library_end) {    \
                    /* Only grab the FIRST 32-byte chunk of the page (contains \
                     * the header) */                                          \
                    if ((w.memory_address % types::page_length) == 0) {        \
                        uint16_t counter;                                      \
                        std::memcpy(&counter, w.data.data() + 2, 2);           \
                        REQUIRE(counter == expected_counter);                  \
                        return w.memory_address;                               \
                    }                                                          \
                }                                                              \
            }                                                                  \
        }                                                                      \
        return static_cast<uint16_t>(0);                                       \
    };

// ============================================================================
// SCENARIO 1: Creation
// ============================================================================
SCENARIO("Book Accessor - Data Partition Creation") {
    auto mock_listener = MockListener{};
    auto buffer = eeprom::book_accessor::DataBufferType<8>();

    auto mock_client =
        BMockEEpromTaskClient<i2c::writer::Writer<test_mocks::MockMessageQueue>,
                              test_mocks::MockI2CResponseQueue>{};
    auto tail_accessor = eeprom::dev_data::DevDataTailAccessor<
        BMockEEpromTaskClient<i2c::writer::Writer<test_mocks::MockMessageQueue>,
                              test_mocks::MockI2CResponseQueue>>{mock_client};
    auto test_book_accessor = book_accessor::BookAccessor<
        BMockEEpromTaskClient<i2c::writer::Writer<test_mocks::MockMessageQueue>,
                              test_mocks::MockI2CResponseQueue>,
        8>{mock_client, mock_listener, buffer, tail_accessor};

    tail_accessor.finish_data_rev();
    test_book_accessor.set_testing(false);
    DEFINE_CHECK_WRITE_HELPER

    GIVEN("An initialized Book Accessor EEPROM") {
        std::array<uint8_t, 8> data_key_0 = {0, 0, 0, 0, 0, 0, 0, 0};
        std::array<uint8_t, 8> data_key_1 = {1, 1, 1, 1, 1, 1, 1, 1};
        uint16_t len = 8;

        THEN(
            "We can create Key 0 and Key 1, checking address separation and "
            "counters") {
            test_book_accessor.create_data_part<8>(0, len, data_key_0);
            uint16_t key_0_address = check_write(1);
            REQUIRE(key_0_address > 0);

            test_book_accessor.create_data_part<8>(1, len, data_key_1);
            uint16_t key_1_address = check_write(1);
            REQUIRE(key_1_address > 0);

            // Verify they are separated by exactly one book length (4 pages)
            REQUIRE(key_0_address - key_1_address ==
                    (types::page_length * types::pages_per_book));
        }
    }
}

// ============================================================================
// SCENARIO 2: Reads, Wrapping, and Cascading
// ============================================================================
SCENARIO("Book Accessor - Reads, Book Wrapping, and CRC Cascade") {
    auto mock_listener = MockListener{};
    auto buffer = eeprom::book_accessor::DataBufferType<8>();

    auto mock_client =
        BMockEEpromTaskClient<i2c::writer::Writer<test_mocks::MockMessageQueue>,
                              test_mocks::MockI2CResponseQueue>{};
    auto tail_accessor = eeprom::dev_data::DevDataTailAccessor<
        BMockEEpromTaskClient<i2c::writer::Writer<test_mocks::MockMessageQueue>,
                              test_mocks::MockI2CResponseQueue>>{mock_client};
    auto test_book_accessor = book_accessor::BookAccessor<
        BMockEEpromTaskClient<i2c::writer::Writer<test_mocks::MockMessageQueue>,
                              test_mocks::MockI2CResponseQueue>,
        8>{mock_client, mock_listener, buffer, tail_accessor};

    tail_accessor.finish_data_rev();
    test_book_accessor.set_testing(false);

    GIVEN("An initialized EEPROM with a created key") {
        uint16_t key = 0;
        uint16_t len = 8;
        uint16_t offset = 0;
        uint32_t message_index = 0;

        std::array<uint8_t, 8> data_c = {0, 0, 0, 0, 0, 0, 0, 0};
        std::array<uint8_t, 8> data_1 = {1, 1, 1, 1, 1, 1, 1, 1};
        std::array<uint8_t, 8> data_2 = {2, 2, 2, 2, 2, 2, 2, 2};
        std::array<uint8_t, 8> data_3 = {3, 3, 3, 3, 3, 3, 3, 3};
        std::array<uint8_t, 8> data_4 = {4, 4, 4, 4, 4, 4, 4, 4};

        test_book_accessor.create_data_part<8>(key, len, data_c);

        THEN("Reads handle page wrapping and CRC cascades successfully") {
            // Read creation (This caches the key and all_reads array)
            test_book_accessor.get_data(key, len, offset, message_index++);
            REQUIRE(buffer[0] == 0);

            // Write 3 times
            test_book_accessor.write_data(key, len, data_1);
            test_book_accessor.write_data(key, len, data_2);
            test_book_accessor.write_data(key, len, data_3);

            // Read to find most recent (Should be Counter 4 / data_3)
            test_book_accessor.get_data(key, len, offset, message_index++);

            REQUIRE(buffer[0] == 3);

            // Write again to trigger a wrap back to Page 0 (Counter 5)
            test_book_accessor.write_data(key, len, data_4);

            // Read to ensure wrap is done properly
            test_book_accessor.get_data(key, len, offset, message_index++);
            REQUIRE(buffer[0] == 4);

            // Cascade Test 1: ONE_INVALID
            mock_client.read_option = ONE_INVALID;
            test_book_accessor.get_data(key, len, offset, message_index++);
            REQUIRE(buffer[0] == 3);

            // Cascade Test 2: ALL_INVALID
            mock_client.read_option = ALL_INVALID;
            test_book_accessor.get_data(key, len, offset, message_index++);
            REQUIRE(buffer[0] == 0xAA);
        }
    }
}

// ============================================================================
// SCENARIO 3: Writes
// ============================================================================
SCENARIO("Book Accessor - Writes") {
    auto mock_listener = MockListener{};
    auto buffer = eeprom::book_accessor::DataBufferType<8>();

    auto mock_client =
        BMockEEpromTaskClient<i2c::writer::Writer<test_mocks::MockMessageQueue>,
                              test_mocks::MockI2CResponseQueue>{};
    auto tail_accessor = eeprom::dev_data::DevDataTailAccessor<
        BMockEEpromTaskClient<i2c::writer::Writer<test_mocks::MockMessageQueue>,
                              test_mocks::MockI2CResponseQueue>>{mock_client};
    auto test_book_accessor = book_accessor::BookAccessor<
        BMockEEpromTaskClient<i2c::writer::Writer<test_mocks::MockMessageQueue>,
                              test_mocks::MockI2CResponseQueue>,
        8>{mock_client, mock_listener, buffer, tail_accessor};

    tail_accessor.finish_data_rev();
    test_book_accessor.set_testing(false);
    DEFINE_CHECK_WRITE_HELPER

    GIVEN("An initialized EEPROM with a created key") {
        uint16_t key = 0;
        uint16_t len = 8;

        std::array<uint8_t, 8> data_c = {0, 0, 0, 0, 0, 0, 0, 0};
        std::array<uint8_t, 8> data_w1 = {1, 1, 1, 1, 1, 1, 1, 1};
        std::array<uint8_t, 8> data_w2 = {2, 2, 2, 2, 2, 2, 2, 2};

        // Note: create_data_part does NOT set cached_key, so these consecutive
        // writes work
        test_book_accessor.create_data_part<8>(key, len, data_c);
        uint16_t base_address = check_write(1);

        THEN(
            "Sequential writes increment counters and format correct memory "
            "addresses") {
            // Write 1
            mock_client.messages_received.clear();
            test_book_accessor.write_data(key, len, data_w1);

            // Should be placed on the next page (offset by 1 page length),
            // counter = 2
            REQUIRE(check_write(2) == base_address + types::page_length);

            // Write 2
            mock_client.messages_received.clear();
            test_book_accessor.write_data(key, len, data_w2);

            // Should be placed on the third page (offset by 2 page lengths),
            // counter = 3
            REQUIRE(check_write(3) == base_address + (2 * types::page_length));
        }
    }
}
