#include <list>
#include <map>
#include <unordered_map>

#include "catch2/catch.hpp"
#include "common/core/allocator.hpp"
#include "common/core/logging.h"

TEST_CASE("Allocator reuses address") {
    auto subject = PoolAllocator<int, 2>{};

    GIVEN("a call allocate.") {
        auto first = subject.allocate(1);

        WHEN("allocated pointer is deallocated.") {
            subject.deallocate(first, 1);
            THEN("the slot is reused") {
                auto second = subject.allocate(1);
                REQUIRE(first == second);
            }
        }
    }
}

TEST_CASE("Deallocate using invalid address") {
    auto subject = PoolAllocator<int, 2>{};

    GIVEN("a call to deallocate.") {
        int i = 0;

        WHEN("deallocating a bad pointer.") {
            subject.deallocate(&i, 1);
            THEN("nothing bad happens") {}
        }
    }

    GIVEN("a call allocate.") {
        auto first = subject.allocate(1);

        WHEN("deallocating a bad address and allocating another element") {
            int i = 0;
            subject.deallocate(&i, 1);
            auto second = subject.allocate(1);

            THEN("the allocated elements have different addresses.") {
                REQUIRE(first != second);
            }
        }
    }
}

TEST_CASE("Ordered Map Object") {
    auto constexpr max_entries = 2;
    std::map<std::string, int, std::less<std::string>,
             PoolAllocator<std::pair<const std::string, int>, max_entries>>
        subject{};
    GIVEN("an entry added to the map") {
        subject["a"] = 12;
        subject["b"] = 2;
        THEN("it should be in the map") {
            REQUIRE(subject["a"] == 12);
            REQUIRE(subject["b"] == 2);
        }
        THEN("we should be able to remove an element") {
            subject.erase("a");
            REQUIRE(subject.size() == 1);
            REQUIRE(!subject.contains("a"));
            REQUIRE(subject.contains("b"));
        }
        THEN("we should not be able to add another element") {
            REQUIRE_THROWS_AS(subject["c"] = 2323, std::bad_alloc);
        }
    }
}

TEST_CASE("List Object") {
    auto constexpr max_entries = 2;
    std::list<int, PoolAllocator<int, max_entries>> subject;
    GIVEN("a vector with max capacity") {
        subject.insert(subject.begin(), 1);
        subject.insert(subject.begin(), 2);
        THEN("adding another element will raise bad_alloc") {
            REQUIRE_THROWS_AS(subject.insert(subject.begin(), 3),
                              std::bad_alloc);
        }
    }
}


TEST_CASE("Vector Object") {
    auto constexpr max_entries = 2;
    std::vector<int, PoolAllocator<int, max_entries>> subject;
    GIVEN("a vector with max capacity") {
        subject.insert(subject.begin(), 1);
        subject.insert(subject.begin(), 2);
        THEN("adding another element will raise bad_alloc") {
            REQUIRE_THROWS_AS(subject.insert(subject.begin(), 3),
                              std::bad_alloc);
        }
    }
}