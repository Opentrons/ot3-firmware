#include <array>

#include "catch2/catch.hpp"
#include "common/tests/mock_message_queue.hpp"
#include "pipettes/core/i2c_writer.hpp"
#include "sensors/tests/callbacks.hpp"

#define u8(X) static_cast<uint8_t>(X)

SCENARIO("Test the i2c command queue writer") {
    test_mocks::MockMessageQueue<i2c_writer::TaskMessage> queue{};
    auto writer = i2c_writer::I2CWriter<test_mocks::MockMessageQueue>{};
    writer.set_queue(&queue);
    i2c_writer::TaskMessage empty_msg{};
    GIVEN("An i2c command queue writer") {
        // Test that different sized buffers can
        // be passed in
        constexpr uint16_t ADDRESS = 0x1;

        WHEN("we write a uint8_t") {
            writer.write(ADDRESS, static_cast<uint8_t>(0x05));
            THEN("the queue has our message and it is serialized") {
                REQUIRE(queue.get_size() == 1);
                queue.try_read(&empty_msg);
                auto msg = std::get<i2c_writer::WriteToI2C>(empty_msg);
                std::array compare{u8(0x05), u8(0), u8(0), u8(0), u8(0)};
                REQUIRE(msg.buffer == compare);
            }
        }
        WHEN("we write a uint32_t") {
            writer.write(ADDRESS, static_cast<uint32_t>(0xd34db33f));
            THEN("the queue has our message and it is serialized") {
                REQUIRE(queue.get_size() == 1);
                queue.try_read(&empty_msg);
                auto msg = std::get<i2c_writer::WriteToI2C>(empty_msg);
                std::array compare{u8(0xd3), u8(0x4d), u8(0xb3), u8(0x3f),
                                   u8(0)};
                REQUIRE(msg.buffer == compare);
            }
        }
        WHEN("we write a two byte buffer") {
            writer.write(ADDRESS, std::array{0x1, 0x02});
            THEN("the queue has our message") {
                REQUIRE(queue.get_size() == 1);
                queue.try_read(&empty_msg);
                auto msg = std::get<i2c_writer::WriteToI2C>(empty_msg);
                std::array compare{u8(0x1), u8(0x2), u8(0), u8(0), u8(0)};
                REQUIRE(msg.buffer == compare);
            }
        }
        WHEN("we write a five byte buffer") {
            writer.write(ADDRESS, std::array{u8(0x01), u8(0x02), u8(0x03),
                                             u8(0x04), u8(0x05)});
            THEN("the queue has our message") {
                REQUIRE(queue.get_size() == 1);
                queue.try_read(&empty_msg);
                auto msg = std::get<i2c_writer::WriteToI2C>(empty_msg);
                std::array compare{u8(0x1), u8(0x2), u8(0x3), u8(0x4), u8(0x5)};
                REQUIRE(msg.buffer == compare);
            }
        }
        WHEN("we try and write a six byte buffer (too big)") {
            writer.write(ADDRESS, std::array{u8(0x01), u8(0x02), u8(0x03),
                                             u8(0x04), u8(0x05), u8(0x06)});
            THEN("the queue has our message") {
                REQUIRE(queue.get_size() == 1);
                queue.try_read(&empty_msg);
                auto msg = std::get<i2c_writer::WriteToI2C>(empty_msg);
                std::array compare{u8(0x1), u8(0x2), u8(0x3), u8(0x4), u8(0x5)};
                REQUIRE(msg.buffer == compare);
            }
        }
        WHEN("we write a register") {
            writer.write(ADDRESS, 0x23, 0xd34db33f);
            THEN("the register is in the buffer correctly") {
                REQUIRE(queue.get_size() == 1);
                queue.try_read(&empty_msg);
                auto msg = std::get<i2c_writer::WriteToI2C>(empty_msg);
                std::array compare{u8(0x23), u8(0xd3), u8(0x4d), u8(0xb3),
                                   u8(0x3f)};
                REQUIRE(msg.buffer == compare);
            }
        }
        WHEN("we read messages") {
            auto callback = mock_callbacks::EmptyCallback{};
            writer.read(ADDRESS, callback, callback);
            writer.read(ADDRESS, callback, callback);
            THEN("the queue has properly formatted read messages") {
                REQUIRE(queue.get_size() == 2);

                queue.try_read(&empty_msg);
                auto read_msg_two_byte =
                    std::get<i2c_writer::ReadFromI2C>(empty_msg);
                auto rb_one = read_msg_two_byte.buffer;
                REQUIRE(std::all_of(rb_one.begin(), rb_one.end(),
                                    [](uint8_t i) { return i == 0; }));

                queue.try_read(&empty_msg);
                auto read_msg_four_byte =
                    std::get<i2c_writer::ReadFromI2C>(empty_msg);
                auto rb_two = read_msg_four_byte.buffer;
                REQUIRE(std::all_of(rb_two.begin(), rb_two.end(),
                                    [](uint8_t i) { return i == 0; }));
            }
        }
    }

    GIVEN("an integer") {
        constexpr uint16_t ADDRESS = 0x1;
        // test add to int and add to buffer
        WHEN("we transact a uint8_t") {
            auto callback = mock_callbacks::EmptyCallback{};
            writer.transact(ADDRESS, static_cast<uint8_t>(0x05), callback,
                            callback);
            THEN("the queue has our message and it is serialized") {
                i2c_writer::TaskMessage empty_msg{};
                REQUIRE(queue.get_size() == 1);
                queue.try_read(&empty_msg);
                auto msg = std::get<i2c_writer::TransactWithI2C>(empty_msg);
                std::array compare{u8(0x05), u8(0), u8(0), u8(0), u8(0)};
                REQUIRE(msg.buffer == compare);
            }
        }
        WHEN("we write a uint32_t") {
            auto callback = mock_callbacks::EmptyCallback{};
            writer.transact(ADDRESS, static_cast<uint32_t>(0xd34db33f),
                            callback, callback);
            THEN("the queue has our message and it is serialized") {
                i2c_writer::TaskMessage empty_msg{};
                REQUIRE(queue.get_size() == 1);
                queue.try_read(&empty_msg);
                auto msg = std::get<i2c_writer::TransactWithI2C>(empty_msg);
                std::array compare{u8(0xd3), u8(0x4d), u8(0xb3), u8(0x3f),
                                   u8(0)};
                REQUIRE(msg.buffer == compare);
            }
        }
        WHEN("we write a two byte buffer") {
            auto callback = mock_callbacks::EmptyCallback{};
            writer.transact(ADDRESS, std::array{0x1, 0x02}, callback, callback);
            THEN("the queue has our message") {
                i2c_writer::TaskMessage empty_msg{};
                REQUIRE(queue.get_size() == 1);
                queue.try_read(&empty_msg);
                auto msg = std::get<i2c_writer::TransactWithI2C>(empty_msg);
                std::array compare{u8(0x1), u8(0x2), u8(0), u8(0), u8(0)};
                REQUIRE(msg.buffer == compare);
            }
        }
        WHEN("we write a five byte buffer") {
            auto callback = mock_callbacks::EmptyCallback{};
            writer.transact(
                ADDRESS,
                std::array{u8(0x01), u8(0x02), u8(0x03), u8(0x04), u8(0x05)},
                callback, callback);
            THEN("the queue has our message") {
                REQUIRE(queue.get_size() == 1);
                queue.try_read(&empty_msg);
                auto msg = std::get<i2c_writer::TransactWithI2C>(empty_msg);
                std::array compare{u8(0x1), u8(0x2), u8(0x3), u8(0x4), u8(0x5)};
                REQUIRE(msg.buffer == compare);
            }
        }
        WHEN("we try and write a six byte buffer (too big)") {
            auto callback = mock_callbacks::EmptyCallback{};
            writer.transact(ADDRESS,
                            std::array{u8(0x01), u8(0x02), u8(0x03), u8(0x04),
                                       u8(0x05), u8(0x06)},
                            callback, callback);
            THEN("the queue has our message") {
                REQUIRE(queue.get_size() == 1);
                queue.try_read(&empty_msg);
                auto msg = std::get<i2c_writer::TransactWithI2C>(empty_msg);

                std::array compare{u8(0x1), u8(0x2), u8(0x3), u8(0x4), u8(0x5)};
                REQUIRE(msg.buffer == compare);
            }
        }
    }
}
