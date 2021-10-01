#include "catch2/catch.hpp"
#include "motor-control/core/motor_interrupt_handler.hpp"

SCENARIO("moving a specific distance in whole steps") {
    TEST_CASE("move a full rotation") {
        GIVEN("") {}
    }
    TEST_CASE("move more than a full rotation") {
        GIVEN("") {}
    }
    TEST_CASE("move multiple full rotations") {}
}

SCENARIO("moving a specific distance in fractional steps") {
    TEST_CASE("") {
        GIVEN("") {}
    }
}

SCENARIO("overflow positional value") {
    TEST_CASE("Move in 10mm increments to largest max X position") {
        // test all of the possible microstepping values here.
        GIVEN("X microstepping") {}
        GIVEN("Y microstepping") {}
    }
}