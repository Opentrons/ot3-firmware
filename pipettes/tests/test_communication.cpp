#include <array>
#include <span>

#include "catch2/catch.hpp"
#include "pipettes/core/communication.hpp"

using namespace communication;

template <typename T, std::size_t L>
struct DataReader {
    DataReader(const std::array<T, L>& buff)
        : buff(buff), iter(this->buff.cbegin()) {}
    void read(std::span<uint8_t> p) {
        auto data = p.data();
        auto length = p.size();

        for (size_t i = 0; i < length; i++) {
            *data++ = *iter++;
        }
    }

  private:
    std::array<T, L> buff;
    typename std::array<T, L>::const_iterator iter;
};

SCENARIO("messages can be parsed") {
    GIVEN("a message reader with a stop message") {
        DataReader dr{std::array{0x0, 0x0, 0, 0}};
        MessageReader r;
        auto response = r.read(dr);

        WHEN("the message is read") {
            THEN("the type should be stop") {
                REQUIRE(
                    std::holds_alternative<pipette_messages::Stop>(response));
            }
        }
    }

    GIVEN("a message reader with a set speed message") {
        DataReader dr{std::array{0, 0, 0, 1, 1, 2, 3, 4}};
        MessageReader r;
        auto response = r.read(dr);

        WHEN("the message is read") {
            THEN("the type should be set speed and the speed should be 1234") {
                REQUIRE(std::holds_alternative<pipette_messages::SetSpeed>(
                    response));
                REQUIRE(std::get<pipette_messages::SetSpeed>(response).mm_sec ==
                        0x01020304);
            }
        }
    }

    GIVEN("a message reader with an unknown message type") {
        DataReader dr{std::array{0xff, 0xff, 0xff, 0xff, 0xff}};
        MessageReader r;
        auto response = r.read(dr);

        WHEN("the message is read") {
            THEN("the response should be empty") {
                REQUIRE(std::holds_alternative<std::monostate>(response));
            }
        }
    }
}

template <typename T, std::size_t L>
struct DataWriter {
    void write(std::span<uint8_t>& p) {
        for (auto i : p) {
            *iter++ = i;
        }
    }

    std::array<T, L> buff{};
    typename std::array<T, L>::iterator iter = buff.begin();
};

SCENARIO("messages can be written") {
    GIVEN("a get speed result message") {
        DataWriter<uint8_t, 8> dw{};
        MessageWriter w;
        auto message = pipette_messages::GetSpeedResult{0xbeef0bad};
        w.write(dw, message);

        WHEN("the message is read") {
            THEN("the type should be stop") {
                REQUIRE(dw.buff[0] == 0);
                REQUIRE(dw.buff[1] == 0);
                REQUIRE(dw.buff[2] == 0);
                REQUIRE(dw.buff[3] == 0x11);
                REQUIRE(dw.buff[4] == 0xbe);
                REQUIRE(dw.buff[5] == 0xef);
                REQUIRE(dw.buff[6] == 0x0b);
                REQUIRE(dw.buff[7] == 0xad);
            }
        }
    }
}