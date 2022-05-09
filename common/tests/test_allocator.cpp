#include <map>
#include <unordered_map>
#include "catch2/catch.hpp"

#include "common/core/allocator.hpp"

TEST_CASE("Ordered Map Object") {
    auto constexpr max_entries=5;
    std::map<std::string, int, std::less<std::string>, PoolAllocator<std::pair<const std::string, int>, max_entries>> subject{};
    GIVEN("an entry added to the map") {
        subject["a"] = 12;
        THEN("it should be in the map") {
            REQUIRE( subject["a"] == 12);
        }
    }
}

TEST_CASE("Unordered Map Object") {

//    std::unordered_map<std::string, int, Allocator=PoolAllocator<200, 5>> task_map {};
}

TEST_CASE("") {

}
