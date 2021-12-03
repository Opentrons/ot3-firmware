#include <map>
#include <unordered_map>
#include "catch2/catch.hpp"

#include "common/core/allocator.hpp"

TEST_CASE("Ordered Map Object") {
    std::map<std::string, int, Allocator=PoolAllocator<200, 5>> task_map {};
}

TEST_CASE("Unordered Map Object") {

    std::unordered_map<std::string, int, Allocator=PoolAllocator<200, 5>> task_map {};
}

TEST_CASE("") {

}
