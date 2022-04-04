#include <map>

#include "catch2/catch.hpp"
#include "common/simulation/i2c_sim.hpp"
#include "common/tests/mock_message_queue.hpp"
#include "pipettes/core/tasks/i2c_poll_task.hpp"
#include "sensors/core/callback_types.hpp"
#include "sensors/simulation/sensors.hpp"
#include "sensors/tests/callbacks.hpp"

#define u8(X) static_cast<uint8_t>(X)

using namespace sensor_callbacks;

SCENARIO("Test the i2c command queue poller") {
    test_mocks::MockMessageQueue<i2c_poller::TaskMessage> queue{};
    auto poller = i2c_poller::I2CPoller<test_mocks::MockMessageQueue>{};
    poller.set_queue(&queue);
    i2c_poller::TaskMessage empty_msg{};
    GIVEN("An i2c command queue poller to do single register limited-polls") {
        // Test that different sized buffers can
        // be passed in
        constexpr uint16_t ADDRESS = 0x1;
        WHEN("we request a poll with the buffer overload") {
            std::array small{u8(0x05)};
            bool client_cb_called = false;
            bool handle_cb_called = false;
            poller.single_register_poll(
                ADDRESS, small, 20, 15,
                [&client_cb_called]() { client_cb_called = true; },
                [&handle_cb_called](const auto& _) {
                    handle_cb_called = true;
                });
            queue.try_read(&empty_msg);
            auto poll_msg =
                std::get<pipette_messages::SingleRegisterPollReadFromI2C>(
                    empty_msg);
            THEN("the callbacks are bound correctly") {
                CHECK(!client_cb_called);
                poll_msg.client_callback();
                REQUIRE(client_cb_called);

                CHECK(!handle_cb_called);
                MaxMessageBuffer fake_cb_payload;
                poll_msg.handle_buffer(fake_cb_payload);
                REQUIRE(handle_cb_called);
            }
            THEN("the other data members are bound correctly") {
                REQUIRE(poll_msg.address == ADDRESS);
                REQUIRE(poll_msg.polling == 20);
                REQUIRE(poll_msg.delay_ms == 15);
            }
        }
        WHEN("we request a poll with the data overload") {
            uint32_t data = 22;
            bool client_cb_called = false;
            bool handle_cb_called = false;
            poller.single_register_poll(
                ADDRESS, data, 20, 15,
                [&client_cb_called]() { client_cb_called = true; },
                [&handle_cb_called](const auto& _) {
                    handle_cb_called = true;
                });
            queue.try_read(&empty_msg);
            auto poll_msg =
                std::get<pipette_messages::SingleRegisterPollReadFromI2C>(
                    empty_msg);
            THEN("the callbacks are bound correctly") {
                CHECK(!client_cb_called);
                poll_msg.client_callback();
                REQUIRE(client_cb_called);

                CHECK(!handle_cb_called);
                MaxMessageBuffer fake_cb_payload{};
                poll_msg.handle_buffer(fake_cb_payload);
                REQUIRE(handle_cb_called);
            }
            THEN("the other data members are bound correctly") {
                REQUIRE(poll_msg.address == ADDRESS);
                REQUIRE(poll_msg.polling == 20);
                REQUIRE(poll_msg.delay_ms == 15);
            }
        }
        WHEN("we request the poll with a short buffer") {
            std::array small{u8(0x05)};
            poller.single_register_poll(
                ADDRESS, small, 20, 9, []() {}, [](const auto& _) {});
            queue.try_read(&empty_msg);
            auto poll_msg =
                std::get<pipette_messages::SingleRegisterPollReadFromI2C>(
                    empty_msg);

            THEN("only the passed-in data is present") {
                REQUIRE(poll_msg.buffer[0] == 0x05);
                auto* iter = poll_msg.buffer.begin();
                iter++;
                for (; iter < poll_msg.buffer.end(); iter++) {
                    REQUIRE(*iter == 0);
                }
            }
        }
        WHEN("we request the poll with a long buffer") {
            std::array small{u8(0x05), u8(0x06), u8(0x07),
                             u8(0x08), u8(0x09), u8(0xa)};
            bool client_cb_called = false;
            bool handle_cb_called = false;
            poller.single_register_poll(
                ADDRESS, small, 20, 15,
                [&client_cb_called]() { client_cb_called = true; },
                [&handle_cb_called](const auto& _) {
                    handle_cb_called = true;
                });
            queue.try_read(&empty_msg);
            auto poll_msg =
                std::get<pipette_messages::SingleRegisterPollReadFromI2C>(
                    empty_msg);

            THEN("passed-in data up to the size limit is present") {
                for (size_t idx = 0; idx < poll_msg.buffer.size(); idx++) {
                    REQUIRE(poll_msg.buffer[idx] == small[idx]);
                }
            }
        }

        WHEN("we write a uint8_t") {
            poller.single_register_poll(
                ADDRESS, static_cast<uint8_t>(0x05), 15, 22, []() {},
                [](const auto& _) {});
            THEN("the queue has our message and it is serialized") {
                REQUIRE(queue.get_size() == 1);
                queue.try_read(&empty_msg);
                auto msg = std::get<i2c_poller::SingleRegisterPollReadFromI2C>(
                    empty_msg);
                std::array compare{u8(0x05), u8(0), u8(0), u8(0), u8(0)};
                REQUIRE(msg.buffer == compare);
            }
        }
        WHEN("we write a uint32_t") {
            poller.single_register_poll(
                ADDRESS, static_cast<uint32_t>(0xd34db33f), 15, 22, []() {},
                [](const auto& _) {});
            THEN("the queue has our message and it is serialized") {
                REQUIRE(queue.get_size() == 1);
                queue.try_read(&empty_msg);
                auto msg = std::get<i2c_poller::SingleRegisterPollReadFromI2C>(
                    empty_msg);
                std::array compare{u8(0xd3), u8(0x4d), u8(0xb3), u8(0x3f),
                                   u8(0)};
                REQUIRE(msg.buffer == compare);
            }
        }
    }

    GIVEN(
        "An i2c command queue poller to do single register continuous-polls") {
        // Test that different sized buffers can
        // be passed in
        constexpr uint16_t ADDRESS = 0x1;
        WHEN("we request a poll with the buffer overload") {
            std::array small{u8(0x05)};
            bool handle_cb_called = false;
            poller.continuous_single_register_poll(
                ADDRESS, small, 20, 15, [&handle_cb_called](const auto& _) {
                    handle_cb_called = true;
                });
            queue.try_read(&empty_msg);
            auto poll_msg = std::get<
                pipette_messages::ConfigureSingleRegisterContinuousPolling>(
                empty_msg);
            THEN("the callbacks are bound correctly") {
                CHECK(!handle_cb_called);
                MaxMessageBuffer fake_cb_payload{};
                poll_msg.handle_buffer(fake_cb_payload);
                REQUIRE(handle_cb_called);
            }
            THEN("the other data members are bound correctly") {
                REQUIRE(poll_msg.address == ADDRESS);
                REQUIRE(poll_msg.delay_ms == 20);
                REQUIRE(poll_msg.poll_id == 15);
            }
        }
        WHEN("we request a poll with the data overload") {
            uint32_t data = 22;
            bool handle_cb_called = false;
            poller.continuous_single_register_poll(
                ADDRESS, data, 20, 15, [&handle_cb_called](const auto& _) {
                    handle_cb_called = true;
                });
            queue.try_read(&empty_msg);
            auto poll_msg = std::get<
                pipette_messages::ConfigureSingleRegisterContinuousPolling>(
                empty_msg);
            THEN("the callbacks are bound correctly") {
                CHECK(!handle_cb_called);
                MaxMessageBuffer fake_cb_payload{};
                poll_msg.handle_buffer(fake_cb_payload);
                REQUIRE(handle_cb_called);
            }
            THEN("the other data members are bound correctly") {
                REQUIRE(poll_msg.address == ADDRESS);
                REQUIRE(poll_msg.delay_ms == 20);
                REQUIRE(poll_msg.poll_id == 15);
            }
        }
        WHEN("we request the poll with a short buffer") {
            std::array small{u8(0x05)};
            poller.continuous_single_register_poll(ADDRESS, small, 20, 15,
                                                   [](const auto& _) {});
            queue.try_read(&empty_msg);
            auto poll_msg = std::get<
                pipette_messages::ConfigureSingleRegisterContinuousPolling>(
                empty_msg);

            THEN("only the passed-in data is present") {
                REQUIRE(poll_msg.buffer[0] == 0x05);
                auto* iter = poll_msg.buffer.begin();
                iter++;
                for (; iter < poll_msg.buffer.end(); iter++) {
                    REQUIRE(*iter == 0);
                }
            }
        }
        WHEN("we request the poll with a long buffer") {
            std::array small{u8(0x05), u8(0x06), u8(0x07),
                             u8(0x08), u8(0x09), u8(0xa)};
            poller.continuous_single_register_poll(ADDRESS, small, 20, 15,
                                                   [](const auto& _) {});
            queue.try_read(&empty_msg);
            auto poll_msg = std::get<
                pipette_messages::ConfigureSingleRegisterContinuousPolling>(
                empty_msg);

            THEN("passed-in data up to the size limit is present") {
                for (size_t idx = 0; idx < poll_msg.buffer.size(); idx++) {
                    REQUIRE(poll_msg.buffer[idx] == small[idx]);
                }
            }
        }

        WHEN("we write a uint8_t") {
            poller.continuous_single_register_poll(
                ADDRESS, static_cast<uint8_t>(0x05), 15, 22,
                [](const auto& _) {});
            THEN("the queue has our message and it is serialized") {
                REQUIRE(queue.get_size() == 1);
                queue.try_read(&empty_msg);
                auto msg = std::get<
                    i2c_poller::ConfigureSingleRegisterContinuousPolling>(
                    empty_msg);
                std::array compare{u8(0x05), u8(0), u8(0), u8(0), u8(0)};
                REQUIRE(msg.buffer == compare);
            }
        }
        WHEN("we write a uint32_t") {
            poller.continuous_single_register_poll(
                ADDRESS, static_cast<uint32_t>(0xd34db33f), 15, 22,
                [](const auto& _) {});
            THEN("the queue has our message and it is serialized") {
                REQUIRE(queue.get_size() == 1);
                queue.try_read(&empty_msg);
                auto msg = std::get<
                    i2c_poller::ConfigureSingleRegisterContinuousPolling>(
                    empty_msg);
                std::array compare{u8(0xd3), u8(0x4d), u8(0xb3), u8(0x3f),
                                   u8(0)};
                REQUIRE(msg.buffer == compare);
            }
        }
    }

    GIVEN("An i2c command queue poller to do multi register limited-polls") {
        // Test that different sized buffers can
        // be passed in
        constexpr uint16_t ADDRESS = 0x1;
        WHEN("we request a poll with the buffer overload") {
            std::array small{u8(0x05)};
            bool handle_cb_called = false;
            bool client_cb_called = false;
            poller.multi_register_poll(
                ADDRESS, small, small, 20, 15,
                [&client_cb_called]() { client_cb_called = true; },
                [&handle_cb_called](const auto& _1, const auto& _2) {
                    handle_cb_called = true;
                });
            queue.try_read(&empty_msg);
            auto poll_msg =
                std::get<pipette_messages::MultiRegisterPollReadFromI2C>(
                    empty_msg);
            THEN("the callbacks are bound correctly") {
                CHECK(!client_cb_called);
                poll_msg.client_callback();
                REQUIRE(client_cb_called);
                CHECK(!handle_cb_called);
                MaxMessageBuffer fake_cb_payload{}, other_fake{};
                poll_msg.handle_buffer(fake_cb_payload, other_fake);
                REQUIRE(handle_cb_called);
            }
            THEN("the other data members are bound correctly") {
                REQUIRE(poll_msg.address == ADDRESS);
                REQUIRE(poll_msg.polling == 20);
                REQUIRE(poll_msg.delay_ms == 15);
            }
        }
        WHEN("we request a poll with the data overload") {
            uint32_t data = 22;
            bool handle_cb_called = false;
            bool client_cb_called = false;
            poller.multi_register_poll(
                ADDRESS, data, data, 20, 15,
                [&client_cb_called]() { client_cb_called = true; },
                [&handle_cb_called](const auto& _1, const auto& _2) {
                    handle_cb_called = true;
                });
            queue.try_read(&empty_msg);
            auto poll_msg =
                std::get<pipette_messages::MultiRegisterPollReadFromI2C>(
                    empty_msg);
            THEN("the callbacks are bound correctly") {
                CHECK(!handle_cb_called);
                MaxMessageBuffer fake_cb_payload_1{}, fake_cb_payload_2{};
                poll_msg.handle_buffer(fake_cb_payload_1, fake_cb_payload_2);
                REQUIRE(handle_cb_called);
            }
            THEN("the other data members are bound correctly") {
                REQUIRE(poll_msg.address == ADDRESS);
                REQUIRE(poll_msg.polling == 20);
                REQUIRE(poll_msg.delay_ms == 15);
            }
        }
        WHEN("we request the poll with two short buffers") {
            std::array small_1{u8(0x05)};
            std::array small_2{u8(0x04)};
            poller.multi_register_poll(
                ADDRESS, small_1, small_2, 20, 15, []() {},
                [](const auto& _1, const auto& _2) {});
            queue.try_read(&empty_msg);
            auto poll_msg =
                std::get<pipette_messages::MultiRegisterPollReadFromI2C>(
                    empty_msg);

            THEN("only the passed-in data is present") {
                std::array check_1{u8(0x05), u8(0), u8(0), u8(0), u8(0)},
                    check_2{u8(0x04), u8(0), u8(0), u8(0), u8(0)};
                REQUIRE(poll_msg.register_1_buffer == check_1);
                REQUIRE(poll_msg.register_2_buffer == check_2);
            }
        }
        WHEN("we request the poll with two long buffers") {
            std::array big_1{u8(0x05), u8(0x06), u8(0x07),
                             u8(0x08), u8(0x09), u8(0x0a)};
            std::array big_2{u8(0x50), u8(0x60), u8(0x70),
                             u8(0x80), u8(0x90), u8(0xa0)};
            poller.multi_register_poll(
                ADDRESS, big_1, big_2, 20, 15, []() {},
                [](const auto& _1, const auto& _2) {});
            queue.try_read(&empty_msg);
            auto poll_msg =
                std::get<pipette_messages::MultiRegisterPollReadFromI2C>(
                    empty_msg);

            THEN("passed-in data up to the size limit is present") {
                for (size_t idx = 0; idx < poll_msg.register_1_buffer.size();
                     idx++) {
                    REQUIRE(poll_msg.register_1_buffer.at(idx) ==
                            big_1.at(idx));
                    REQUIRE(poll_msg.register_2_buffer.at(idx) ==
                            big_2.at(idx));
                }
            }
        }

        WHEN("we write uint8_ts") {
            uint8_t first = 10, second = 15;
            poller.multi_register_poll(
                ADDRESS, first, second, 15, 22, []() {},
                [](const auto& _1, const auto& _2) {});
            REQUIRE(queue.get_size() == 1);
            queue.try_read(&empty_msg);
            auto msg =
                std::get<i2c_poller::MultiRegisterPollReadFromI2C>(empty_msg);

            THEN("the queue has our message and it is serialized") {
                std::array compare_1{u8(10), u8(0), u8(0), u8(0), u8(0)},
                    compare_2{u8(15), u8(0), u8(0), u8(0), u8(0)};
                REQUIRE(msg.register_1_buffer == compare_1);
                REQUIRE(msg.register_2_buffer == compare_2);
            }
        }
        WHEN("we write uint32_ts") {
            uint32_t first = 0xd34db33f, second = 0xc0ffee55;
            poller.multi_register_poll(
                ADDRESS, first, second, 15, 22, []() {},
                [](const auto& _1, const auto& _2) {});
            REQUIRE(queue.get_size() == 1);
            queue.try_read(&empty_msg);
            auto msg =
                std::get<i2c_poller::MultiRegisterPollReadFromI2C>(empty_msg);

            THEN("the queue has our message and it is serialized") {
                std::array compare_1{u8(0xd3), u8(0x4d), u8(0xb3), u8(0x3f),
                                     u8(0)};
                std::array compare_2{u8(0xc0), u8(0xff), u8(0xee), u8(0x55),
                                     u8(0)};
                REQUIRE(msg.register_1_buffer == compare_1);
                REQUIRE(msg.register_2_buffer == compare_2);
            }
        }
    }

    GIVEN("An i2c command queue poller to do multi register continuous polls") {
        // Test that different sized buffers can
        // be passed in
        constexpr uint16_t ADDRESS = 0x1;
        WHEN("we request a poll with the buffer overload") {
            std::array small{u8(0x05)};
            bool handle_cb_called = false;
            poller.continuous_multi_register_poll(
                ADDRESS, small, small, 20, 15,
                [&handle_cb_called](const auto& _1, const auto& _2) {
                    handle_cb_called = true;
                });
            queue.try_read(&empty_msg);
            auto poll_msg = std::get<
                pipette_messages::ConfigureMultiRegisterContinuousPolling>(
                empty_msg);
            THEN("the callbacks are bound correctly") {
                CHECK(!handle_cb_called);
                MaxMessageBuffer fake_cb_payload{}, other_fake{};
                poll_msg.handle_buffer(fake_cb_payload, other_fake);
                REQUIRE(handle_cb_called);
            }
            THEN("the other data members are bound correctly") {
                REQUIRE(poll_msg.address == ADDRESS);
                REQUIRE(poll_msg.delay_ms == 20);
                REQUIRE(poll_msg.poll_id == 15);
            }
        }
        WHEN("we request a poll with the data overload") {
            uint32_t data = 22;
            bool handle_cb_called = false;
            poller.continuous_multi_register_poll(
                ADDRESS, data, data, 20, 15,
                [&handle_cb_called](const auto& _1, const auto& _2) {
                    handle_cb_called = true;
                });
            queue.try_read(&empty_msg);
            auto poll_msg = std::get<
                pipette_messages::ConfigureMultiRegisterContinuousPolling>(
                empty_msg);
            THEN("the callbacks are bound correctly") {
                CHECK(!handle_cb_called);
                MaxMessageBuffer fake_cb_payload_1{}, fake_cb_payload_2{};
                poll_msg.handle_buffer(fake_cb_payload_1, fake_cb_payload_2);
                REQUIRE(handle_cb_called);
            }
            THEN("the other data members are bound correctly") {
                REQUIRE(poll_msg.address == ADDRESS);
                REQUIRE(poll_msg.delay_ms == 20);
                REQUIRE(poll_msg.poll_id == 15);
            }
        }
        WHEN("we request the poll with two short buffers") {
            std::array small_1{u8(0x05)};
            std::array small_2{u8(0x04)};
            poller.continuous_multi_register_poll(
                ADDRESS, small_1, small_2, 20, 15,
                [](const auto& _1, const auto& _2) {});
            queue.try_read(&empty_msg);
            auto poll_msg = std::get<
                pipette_messages::ConfigureMultiRegisterContinuousPolling>(
                empty_msg);

            THEN("only the passed-in data is present") {
                REQUIRE(poll_msg.register_1_buffer[0] == 0x05);
                REQUIRE(poll_msg.register_2_buffer[0] == 0x04);
                for (size_t idx = 1; idx < poll_msg.register_1_buffer.size();
                     idx++) {
                    REQUIRE(poll_msg.register_1_buffer.at(idx) == 0);
                    REQUIRE(poll_msg.register_2_buffer.at(idx) == 0);
                }
            }
        }
        WHEN("we request the poll with two long buffers") {
            std::array big_1{u8(0x05), u8(0x06), u8(0x07),
                             u8(0x08), u8(0x09), u8(0x0a)};
            std::array big_2{u8(0x50), u8(0x60), u8(0x70),
                             u8(0x80), u8(0x90), u8(0xa0)};
            poller.continuous_multi_register_poll(
                ADDRESS, big_1, big_2, 20, 15,
                [](const auto& _1, const auto& _2) {});
            queue.try_read(&empty_msg);
            auto poll_msg = std::get<
                pipette_messages::ConfigureMultiRegisterContinuousPolling>(
                empty_msg);

            THEN("passed-in data up to the size limit is present") {
                for (size_t idx = 0; idx < poll_msg.register_1_buffer.size();
                     idx++) {
                    REQUIRE(poll_msg.register_1_buffer.at(idx) ==
                            big_1.at(idx));
                    REQUIRE(poll_msg.register_2_buffer.at(idx) ==
                            big_2.at(idx));
                }
            }
        }

        WHEN("we write uint8_ts") {
            uint8_t first = 10, second = 15;
            poller.continuous_multi_register_poll(
                ADDRESS, first, second, 15, 22,
                [](const auto& _1, const auto& _2) {});
            REQUIRE(queue.get_size() == 1);
            queue.try_read(&empty_msg);
            auto msg =
                std::get<i2c_poller::ConfigureMultiRegisterContinuousPolling>(
                    empty_msg);

            THEN("the queue has our message and it is serialized") {
                std::array compare_1{u8(10), u8(0), u8(0), u8(0), u8(0)},
                    compare_2{u8(15), u8(0), u8(0), u8(0), u8(0)};
                REQUIRE(msg.register_1_buffer == compare_1);
                REQUIRE(msg.register_2_buffer == compare_2);
            }
        }
        WHEN("we write uint32_ts") {
            uint32_t first = 0xd34db33f, second = 0xc0ffee55;
            poller.continuous_multi_register_poll(
                ADDRESS, first, second, 15, 22,
                [](const auto& _1, const auto& _2) {});
            REQUIRE(queue.get_size() == 1);
            queue.try_read(&empty_msg);
            auto msg =
                std::get<i2c_poller::ConfigureMultiRegisterContinuousPolling>(
                    empty_msg);

            THEN("the queue has our message and it is serialized") {
                std::array compare_1{u8(0xd3), u8(0x4d), u8(0xb3), u8(0x3f),
                                     u8(0)};
                std::array compare_2{u8(0xc0), u8(0xff), u8(0xee), u8(0x55),
                                     u8(0)};
                REQUIRE(msg.register_1_buffer == compare_1);
                REQUIRE(msg.register_2_buffer == compare_2);
            }
        }
    }
}
