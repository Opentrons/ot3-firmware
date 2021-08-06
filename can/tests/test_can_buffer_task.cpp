#include <array>

#include "can/core/can_buffer_task.hpp"
#include "catch2/catch.hpp"

using namespace can_message_buffer;

SCENARIO("can message buffer writer send works") {
    struct MockMessageBuffer {
        auto send(const uint8_t* buffer, uint32_t buffer_length,
                  uint32_t timeout) -> std::size_t {
            for (uint32_t i = 0; i < buffer_length; i++) {
                buff[i] = *(buffer + i);
            }
            this->timeout = timeout;
            length = buffer_length;
            return buffer_length;
        }

        auto send_from_isr(const uint8_t* buffer, uint32_t buffer_length)
            -> std::size_t {
            return 0;
        }
        auto receive(uint8_t* buffer, uint32_t buffer_length, uint32_t timeout)
            -> std::size_t {
            return 0;
        }

        std::array<uint32_t, 68> buff;
        uint32_t length;
        uint32_t timeout;
    };

    auto buffer = MockMessageBuffer();
    auto subject = CanMessageBufferWriter(buffer);

    GIVEN("a message and arbitration id") {
        WHEN("sent") {
            auto body = std::array<uint8_t, 2>{0, 1};
            auto arbitration_id = 0x332211ff;
            uint32_t timeout = 100;
            auto r =
                subject.send(arbitration_id, body.begin(), body.end(), timeout);
            THEN("it is serialized correctly") {
                REQUIRE(buffer.buff[0] == 0x33);
                REQUIRE(buffer.buff[1] == 0x22);
                REQUIRE(buffer.buff[2] == 0x11);
                REQUIRE(buffer.buff[3] == 0xff);
                REQUIRE(buffer.buff[4] == 0);
                REQUIRE(buffer.buff[5] == 1);
                REQUIRE(buffer.length == 6);
                REQUIRE(r);
                REQUIRE(buffer.timeout == timeout);
            }
        }
    }

    GIVEN("an arbitration id") {
        WHEN("sent") {
            auto body = std::array<uint8_t, 0>{};
            auto arbitration_id = 0x332211ff;
            uint32_t timeout = 1;
            auto r =
                subject.send(arbitration_id, body.begin(), body.end(), timeout);
            THEN("it is serialized correctly") {
                REQUIRE(buffer.buff[0] == 0x33);
                REQUIRE(buffer.buff[1] == 0x22);
                REQUIRE(buffer.buff[2] == 0x11);
                REQUIRE(buffer.buff[3] == 0xff);
                REQUIRE(buffer.length == 4);
                REQUIRE(r);
                REQUIRE(buffer.timeout == timeout);
            }
        }
    }
}

SCENARIO("can message buffer writer send_from_isr works") {
    struct MockMessageBuffer {
        auto send_from_isr(const uint8_t* buffer, uint32_t buffer_length)
            -> std::size_t {
            for (uint32_t i = 0; i < buffer_length; i++) {
                buff[i] = *(buffer + i);
            }
            length = buffer_length;
            return buffer_length;
        }

        auto send(const uint8_t* buffer, uint32_t buffer_length,
                  uint32_t timeout) -> std::size_t {
            return 0;
        }
        auto receive(uint8_t* buffer, uint32_t buffer_length, uint32_t timeout)
            -> std::size_t {
            return 0;
        }

        std::array<uint32_t, 68> buff;
        uint32_t length;
    };

    auto buffer = MockMessageBuffer();
    auto subject = CanMessageBufferWriter(buffer);

    GIVEN("a message and arbitration id") {
        WHEN("sent") {
            auto body = std::array<uint8_t, 2>{0, 1};
            auto arbitration_id = 0x332211ff;
            auto r =
                subject.send_from_isr(arbitration_id, body.begin(), body.end());
            THEN("it is serialized correctly") {
                REQUIRE(buffer.buff[0] == 0x33);
                REQUIRE(buffer.buff[1] == 0x22);
                REQUIRE(buffer.buff[2] == 0x11);
                REQUIRE(buffer.buff[3] == 0xff);
                REQUIRE(buffer.buff[4] == 0);
                REQUIRE(buffer.buff[5] == 1);
                REQUIRE(buffer.length == 6);
                REQUIRE(r);
            }
        }
    }

    GIVEN("an arbitration id") {
        WHEN("sent") {
            auto body = std::array<uint8_t, 0>{};
            auto arbitration_id = 0x332211ff;
            auto r =
                subject.send_from_isr(arbitration_id, body.begin(), body.end());
            THEN("it is serialized correctly") {
                REQUIRE(buffer.buff[0] == 0x33);
                REQUIRE(buffer.buff[1] == 0x22);
                REQUIRE(buffer.buff[2] == 0x11);
                REQUIRE(buffer.buff[3] == 0xff);
                REQUIRE(buffer.length == 4);
                REQUIRE(r);
            }
        }
    }
}

SCENARIO("can message buffer reader read works") {
    struct MockMessageBuffer {
        MockMessageBuffer(const uint8_t* buff, uint32_t length)
            : length{length} {
            for (uint32_t i = 0; i < length; i++) {
                buffer[i] = *(buff + i);
            }
        }

        auto send_from_isr(const uint8_t* buffer, uint32_t buffer_length)
            -> std::size_t {
            return 0;
        }

        auto send(const uint8_t* buffer, uint32_t buffer_length,
                  uint32_t timeout) -> std::size_t {
            return 0;
        }
        auto receive(uint8_t* buffer, uint32_t buffer_length, uint32_t timeout)
            -> std::size_t {
            for (uint32_t i = 0; i < length; i++) {
                *(buffer + i) = this->buffer[i];
            }
            this->timeout = timeout;
            return buffer_length;
        }

        std::array<uint8_t, 68> buffer{};
        uint32_t length = 0;
        uint32_t timeout = 0;
    };

    struct Listener {
        void handle(uint32_t arbitration_id,
                    std::array<uint8_t, 68>::iterator start,
                    std::array<uint8_t, 68>::iterator limit) {
            this->arbitration_id = arbitration_id;
            auto iter = buff.begin();
            for (; start < limit && iter < buff.end(); start++, iter++) {
                *iter = *start;
            }
        }

        uint32_t arbitration_id = 0;
        std::array<uint8_t, 64> buff{};
    };

    GIVEN("a buffer with an arbitration id and message") {
        WHEN("read") {
            auto body = std::array<uint8_t, 8>{1, 2, 3, 4, 5, 6, 7, 8};
            auto listener = Listener();
            auto buffer = MockMessageBuffer(body.data(), body.size());
            auto subject = CanMessageBufferReader(buffer, listener);

            subject.read(100);
            THEN("it is serialized correctly") {
                REQUIRE(buffer.timeout == 100);
                REQUIRE(listener.arbitration_id == 0x01020304);
                REQUIRE(listener.buff[0] == 0x05);
                REQUIRE(listener.buff[1] == 0x06);
                REQUIRE(listener.buff[2] == 0x07);
                REQUIRE(listener.buff[3] == 0x08);
            }
        }
    }

    GIVEN("a buffer with an arbitration id") {
        WHEN("read") {
            auto body = std::array<uint8_t, 4>{0xff, 0xfe, 0xfd, 0xfc};
            auto listener = Listener();
            auto buffer = MockMessageBuffer(body.data(), body.size());
            auto subject = CanMessageBufferReader(buffer, listener);

            subject.read(100);
            THEN("it is serialized correctly") {
                REQUIRE(buffer.timeout == 100);
                REQUIRE(listener.arbitration_id == 0xfffefdfc);
                REQUIRE(listener.buff == std::array<uint8_t, 64>{});
            }
        }
    }
}