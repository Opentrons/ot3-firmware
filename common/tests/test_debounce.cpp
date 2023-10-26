#include <atomic>

#include "catch2/catch.hpp"
#include "common/core/debounce.hpp"

SCENARIO("debouncing gpio pins works") {
    GIVEN("a state value and bounce value") {
        debouncer::Debouncer subject;
        WHEN("switching from unset to set") {
            subject.debounce_update(true);
            THEN("state stays the same on the first tick") {
                REQUIRE(subject.debounce_state() == false);
            }
            subject.debounce_update(true);
            THEN("state updates on the second tick") {
                REQUIRE(subject.debounce_state() == true);
            }
        }
        WHEN("switching from set to unset") {
            // make sure the state is true
            subject.debounce_update(true);
            subject.debounce_update(true);

            subject.debounce_update(false);
            THEN("state stays the same on the first tick") {
                REQUIRE(subject.debounce_state() == true);
            }
            subject.debounce_update(false);
            THEN("state updates on the second tick") {
                REQUIRE(subject.debounce_state() == false);
            }
        }
    }
}

SCENARIO("debouncing with holdoff cnt works") {
    GIVEN("a state value and bounce value") {
        debouncer::Debouncer subject{.holdoff_cnt = 2};
        WHEN("switching from unset to set") {
            subject.debounce_update(true);
            CHECK(subject.cnt == 0);
            subject.debounce_update(true);
            CHECK(subject.cnt == 1);
            THEN("state stays the same on the first two ticks") {
                REQUIRE(subject.debounce_state() == false);
            }
            subject.debounce_update(true);
            THEN("state updates on the third tick") {
                REQUIRE(subject.debounce_state() == true);
                THEN("cnt gets reset") { CHECK(subject.cnt == 0); }
            }
            WHEN("reset is called") {
                subject.reset();
                THEN("values are reset") {
                    REQUIRE(subject.cnt == 0);
                    REQUIRE(subject.state_bounce == false);
                    REQUIRE(subject.state == false);
                    // holdoff cnt remains the same
                    REQUIRE(subject.holdoff_cnt == 2);
                }
            }
        }
        WHEN("switching from set to unset") {
            // make sure the state is true
            subject.debounce_update(true);
            subject.debounce_update(true);
            subject.debounce_update(true);

            subject.debounce_update(false);
            subject.debounce_update(false);
            THEN("state stays the same on the first two ticks") {
                REQUIRE(subject.debounce_state() == true);
            }
            subject.debounce_update(false);
            THEN("state updates on the third tick") {
                REQUIRE(subject.debounce_state() == false);
            }
        }
        WHEN("switching from set to unset before holdoff cnt is reached") {
            // make sure the state is true
            subject.debounce_update(true);
            subject.debounce_update(true);
            CHECK(subject.cnt == 1);
            subject.debounce_update(false);
            THEN("cnt gets reset back to 0") {
                CHECK(subject.cnt == 0);
                REQUIRE(subject.debounce_state() == false);
            }
            THEN("it requires another 3 updates to actually update the state") {
                subject.debounce_update(true);
                CHECK(subject.cnt == 0);
                REQUIRE(subject.debounce_state() == false);
                subject.debounce_update(true);
                CHECK(subject.cnt == 1);
                REQUIRE(subject.debounce_state() == false);
                subject.debounce_update(true);
                CHECK(subject.cnt == 0);
                REQUIRE(subject.debounce_state() == true);
            }
        }
    }
}
