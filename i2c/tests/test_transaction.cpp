#include "catch2/catch.hpp"
#include "i2c/core/transaction.hpp"


SCENARIO("id map") {
    GIVEN("an IdMap object with capacity of 1") {
        auto subject = i2c::transaction::IdMap<int, 1>{};

        WHEN("adding an entry") {
            auto id = subject.add(2);
            THEN("an id is returned") {
                REQUIRE(id);
            }
        }

        WHEN("adding too many entries") {
            auto id1 = subject.add(2);
            auto id2 = subject.add(2);
            THEN("first id is good") {
                REQUIRE(id1);
            }
            THEN("second id is not set") {
                REQUIRE(!id2);
            }
        }

        WHEN("adding then clearing an entry") {
            auto id = subject.add(52);
            auto value = subject.remove(id.value());
            THEN("the value is returned by clear") {
                REQUIRE(value.value()==52);
            }
        }

        WHEN("remove a non existent id") {
            auto value = subject.remove(0);
            THEN("the return is not set") {
                REQUIRE(!value);
            }
        }

        WHEN("removing out of range") {
            auto value = subject.remove(10020);
            THEN("the return is not set") {
                REQUIRE(!value);
            }
        }
    }

    GIVEN("an IdMap object") {
        auto subject = i2c::transaction::IdMap<int, 5>{};

        THEN("the count is 0") {
            REQUIRE(subject.count()==0);
        }

        WHEN("adding two entries") {
            auto id1 = subject.add(2);
            auto id2 = subject.add(2);

            THEN("the count is 2") { REQUIRE(subject.count()==2); }
            WHEN("removing one") {
                subject.remove(id2.value());
                THEN("the count is 1") { REQUIRE(subject.count()==1); }
            }
            WHEN("removing another both") {
                subject.remove(id2.value());
                subject.remove(id1.value());
                THEN("the count is 0") { REQUIRE(subject.count()==0); }
            }
        }
    }
}
