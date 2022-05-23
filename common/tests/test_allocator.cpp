#include <map>
#include <unordered_map>
#include "catch2/catch.hpp"
#include "common/core/logging.h"
#include "common/core/allocator.hpp"

TEST_CASE("Ordered Map Object") {
    auto constexpr max_entries=5;
    std::map<std::string, int, std::less<std::string>, PoolAllocator<std::pair<const std::string, int>, max_entries>> subject{};
    GIVEN("an entry added to the map") {
        subject["a"] = 12;
        subject["b"] = 2;
        THEN("it should be in the map") {
            REQUIRE( subject["a"] == 12);
            REQUIRE( subject["b"] == 2);

        }
        THEN("we should be able to remove an element") {
            subject.erase("a");
            REQUIRE(subject.size() == 1);
            REQUIRE(!subject.contains("a"));
            REQUIRE(subject.contains("b"));
        }
    }
}

TEST_CASE("Vector Object") {
//    auto constexpr max_entries=5;
//    std::vector subject{1, 2, 3, 4, 5};
//    std::vector expected{1, 2, 3, 4, 5};
////    std::vector<int, PoolAllocator<int, max_entries>> subject{1, 2, 3, 4, 5};
//    GIVEN("too many elements in a vector") {
//        subject.push_back(6);
//
//    }
}

TEST_CASE("") {

}
