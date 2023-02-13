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
