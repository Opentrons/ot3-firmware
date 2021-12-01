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
        subject.node_id(0xFFFF);
        subject.function_code(0xFFFF);
        subject.message_id(0xFFFF);

        WHEN("converted to an integer") {
            THEN("All 29-bits are set") { REQUIRE(subject == 0x1FFFFFFF); }
        }
    }
}
