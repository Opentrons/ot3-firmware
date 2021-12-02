#include "can/core/arbitration_id.hpp"
#include "catch2/catch.hpp"

using namespace can_arbitration_id;

SCENARIO("Arbitration ID from integer") {
    GIVEN("an integer where all bits are set") {
        auto arbitration_id = 0xFFFFFFFF;
        WHEN("creating an arbitration id") {
            auto subject = ArbitrationId(arbitration_id);
            THEN("the individual parts are assigned correctly") {
                REQUIRE(subject.node_id() == 0xFF);
                REQUIRE(subject.function_code() == 0x7F);
                REQUIRE(subject.message_id() == 0x3FFF);
            }
        }
    }
}

SCENARIO("Arbitration ID to integer") {
    GIVEN("an arbitration id object with all bits set") {
        auto subject = ArbitrationId();
        subject.node_id(0xFF);
        subject.function_code(0xFF);
        subject.message_id(0xFFFF);

        WHEN("converted to an integer") {
            THEN("All 29-bits are set") { REQUIRE(subject == 0x1FFFFFFF); }
        }
    }
}

SCENARIO("Arbitration ID") {
    GIVEN("an arbitration id object") {
        auto subject = ArbitrationId();
        subject.node_id(123);
        subject.function_code(210);
        subject.message_id(4321);

        WHEN("converted to an integer") {
            THEN("the bits are set correctly") {
                REQUIRE(subject == ((4321 << 15) | (123 << 7) | 210));
            }
        }

        WHEN("creating another arbitration id from integer") {
            auto subject2 = ArbitrationId(subject);
            THEN("the individual values are the same") {
                REQUIRE(subject.node_id() == subject2.node_id());
                REQUIRE(subject.function_code() == subject2.function_code());
                REQUIRE(subject.message_id() == subject2.message_id());
            }
        }
    }
}
