#include <array>

#include "can/core/can_message_buffer.hpp"
#include "can/tests/mock_message_buffer.hpp"
#include "catch2/catch.hpp"

using namespace can::message_buffer;
using namespace mock_message_buffer;

SCENARIO("can message buffer writer send works") {
    using TestBufferType = std::array<uint8_t, 2>;

    auto buffer = MockMessageBuffer<2>();
    auto subject = CanMessageBufferWriter(buffer);

    GIVEN("a message and arbitration id") {
        WHEN("sent") {
            auto body = TestBufferType{0, 1};
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
            auto body = TestBufferType{};
            auto arbitration_id = 0x332211ff;
            uint32_t timeout = 1;
            auto r = subject.send(arbitration_id, body.begin(), body.begin(),
                                  timeout);
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
    using TestBufferType = std::array<uint8_t, 2>;

    auto buffer = MockMessageBuffer<2>();
    auto subject = CanMessageBufferWriter(buffer);

    GIVEN("a message and arbitration id") {
        WHEN("sent") {
            auto body = TestBufferType{0, 1};
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
            auto body = TestBufferType{};
            auto arbitration_id = 0x332211ff;
            auto r = subject.send_from_isr(arbitration_id, body.begin(),
                                           body.begin());
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
    using TestBufferType = std::array<uint8_t, 8>;

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
            auto body = TestBufferType{1, 2, 3, 4, 5, 6, 7, 8};
            auto listener = Listener();
            auto buffer = MockMessageBuffer<8>(body.begin(), body.end());
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
            auto body = TestBufferType{0xff, 0xfe, 0xfd, 0xfc};
            auto listener = Listener();
            auto buffer = MockMessageBuffer<4>(body.begin(), body.begin() + 4);
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