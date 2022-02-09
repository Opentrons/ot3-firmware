#include "catch2/catch.hpp"
#include "common/tests/mock_message_queue.hpp"
#include "pipettes/core/i2c_writer.hpp"

SCENARIO("Test the i2c command queue writer") {
    test_mocks::MockMessageQueue<i2c_writer::TaskMessage> queue{};
    i2c_writer::TaskMessage empty_msg{};
    auto writer = i2c_writer::I2CWriter<test_mocks::MockMessageQueue>{};
    writer.set_queue(&queue);

    std::array<uint8_t, 2> two_byte_arr{};
    std::array<uint8_t, 5> four_byte_arr{};

    uint16_t two_byte_data = 0x2;
    uint32_t four_byte_data = 0x5;

    GIVEN("Differently sized byte arrays") {
        // Test that different sized buffers can
        // be passed in
        constexpr uint16_t ADDRESS = 0x1;
        auto callback = [](const uint8_t *, const uint16_t) -> void {};

        WHEN("we write messages") {
            writer.write(two_byte_arr, two_byte_data, ADDRESS);
            writer.write(four_byte_arr, four_byte_data, ADDRESS);
            THEN(
                "the queue has two items in it and they are the messages we "
                "wrote") {
                REQUIRE(queue.get_size() == 2);

                queue.try_read(&empty_msg);
                auto write_msg_four_byte =
                    std::get<i2c_writer::WriteToI2C>(empty_msg);
                REQUIRE(write_msg_four_byte.size == four_byte_arr.size());
                REQUIRE(write_msg_four_byte.buffer == four_byte_arr.data());

                queue.try_read(&empty_msg);
                auto write_msg_two_byte =
                    std::get<i2c_writer::WriteToI2C>(empty_msg);
                REQUIRE(write_msg_two_byte.size == two_byte_arr.size());
                REQUIRE(write_msg_two_byte.buffer == two_byte_arr.data());
            }
        }
        WHEN("we read messages") {
            writer.read(four_byte_arr, ADDRESS, callback);
            writer.read(two_byte_arr, ADDRESS, callback);
            THEN("the queue has properly formatted read messages") {
                REQUIRE(queue.get_size() == 2);

                queue.try_read(&empty_msg);
                auto read_msg_two_byte =
                    std::get<i2c_writer::ReadFromI2C>(empty_msg);
                REQUIRE(read_msg_two_byte.buffer == two_byte_arr.data());

                queue.try_read(&empty_msg);
                auto read_msg_four_byte =
                    std::get<i2c_writer::ReadFromI2C>(empty_msg);
                REQUIRE(read_msg_four_byte.buffer == four_byte_arr.data());
            }
        }
    }

    GIVEN("an integer") {
        // test add to int and add to buffer
        writer.add_to_buffer(two_byte_arr, two_byte_data);
        THEN("turn it into a byte array") {
            REQUIRE(two_byte_arr[1] == two_byte_data);
        }
    }
    GIVEN("a byte buffer of data") {
        four_byte_arr[3] = 5;
        // test add to int and add to buffer
        THEN("turn that buffer into an integer") {
            uint32_t new_integer = 0x0;
            writer.add_to_int(four_byte_arr, new_integer);
            REQUIRE(new_integer == four_byte_data);
        }
    }
}