#include "catch2/catch.hpp"
#include "common/tests/mock_message_queue.hpp"
#include "pipettes/core/i2c_writer.hpp"
#include "sensors/tests/callbacks.hpp"

SCENARIO("Test the i2c command queue writer") {
    test_mocks::MockMessageQueue<i2c_writer::TaskMessage> queue{};
    i2c_writer::TaskMessage empty_msg{};
    auto writer = i2c_writer::I2CWriter<test_mocks::MockMessageQueue>{};
    writer.set_queue(&queue);

    uint16_t two_byte_data = 0x2;
    uint32_t four_byte_data = 0x5;
    constexpr auto MAX_SIZE = 5;

    GIVEN("An i2c command queue writer") {
        // Test that different sized buffers can
        // be passed in
        constexpr uint16_t ADDRESS = 0x1;

        WHEN("we write messages") {
            writer.write(two_byte_data, ADDRESS);
            writer.write(four_byte_data, ADDRESS);
            THEN(
                "the queue has two items in it and they are the messages we "
                "wrote") {
                REQUIRE(queue.get_size() == 2);

                queue.try_read(&empty_msg);
                auto write_msg_four_byte =
                    std::get<i2c_writer::WriteToI2C>(empty_msg);
                auto wb_one = write_msg_four_byte.buffer;
                REQUIRE(wb_one.size() == MAX_SIZE);
                REQUIRE(wb_one[3] == four_byte_data);

                queue.try_read(&empty_msg);
                auto write_msg_two_byte =
                    std::get<i2c_writer::WriteToI2C>(empty_msg);
                auto wb_two = write_msg_two_byte.buffer;
                REQUIRE(wb_two.size() == MAX_SIZE);
                REQUIRE(wb_two[1] == two_byte_data);
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
                REQUIRE(rb_one.size() == MAX_SIZE);
                REQUIRE(std::all_of(rb_one.begin(), rb_one.end(),
                                    [](uint8_t i) { return i == 0; }));

                queue.try_read(&empty_msg);
                auto read_msg_four_byte =
                    std::get<i2c_writer::ReadFromI2C>(empty_msg);
                auto rb_two = read_msg_four_byte.buffer;
                REQUIRE(rb_two.size() == MAX_SIZE);
                REQUIRE(std::all_of(rb_two.begin(), rb_two.end(),
                                    [](uint8_t i) { return i == 0; }));
            }
        }
    }

    std::array<uint8_t, 3> three_byte_arr{};
    std::array<uint8_t, 5> four_byte_arr{};
    constexpr uint8_t reg = 0x1;
    GIVEN("an integer") {
        // test add to int and add to buffer
        writer.buffering(three_byte_arr, two_byte_data, reg);
        THEN("turn it into a byte array") {
            REQUIRE(three_byte_arr[0] == reg);
            REQUIRE(three_byte_arr[2] == two_byte_data);
        }
    }
    GIVEN("a byte buffer of data") {
        four_byte_arr[3] = 5;
        // test add to int and add to buffer
        THEN("turn that buffer into an integer") {
            uint32_t new_integer = 0x0;
            writer.debuffering(four_byte_arr, new_integer);
            REQUIRE(new_integer == four_byte_data);
        }
    }
}