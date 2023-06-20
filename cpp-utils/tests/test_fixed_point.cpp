#include "catch2/catch.hpp"
#include "ot_utils/core/fixed_point.hpp"

using namespace ot_utils::fixed_point;

template<class integer_t>
auto convert_to_integer(float f, int conversion) {
    return static_cast<integer_t>(f * static_cast<float>(1LL << conversion));
}


SCENARIO("Fixed point multiplication") {
    GIVEN("Two integers") {
        WHEN("both are positive") {
            int32_t a = convert_to_integer<int32_t>(0.3, 31);
            int32_t b = convert_to_integer<int32_t>(0.5, 31);
            auto result = fixed_point_multiply<int32_t>(a, b, 31);
            THEN("the result should be 0.15") {
                int32_t expected = convert_to_integer<int32_t>(0.15, 31);
                REQUIRE(result == expected);
            }
        }
        WHEN("one is positive and one is negative") {
            int32_t a = convert_to_integer<int32_t>(0.5, 31);
            int32_t b = convert_to_integer<int32_t>(-0.75, 31);

            auto result = fixed_point_multiply<int32_t>(a, b, 31);
            THEN("the result should be -0.375") {
                int32_t expected = convert_to_integer<int32_t>(-0.375, 31);
                REQUIRE(result == expected);
            }
        }
        WHEN("both are negative") {
            int32_t a = convert_to_integer<int32_t>(-0.25, 31);
            int32_t b = convert_to_integer<int32_t>(-0.25, 31);

            auto result = fixed_point_multiply<int32_t>(a, b, 31);
            THEN("the result should be 0.4") {
                int32_t expected = convert_to_integer<int32_t>(0.0625, 31);
                REQUIRE(result == expected);
            }
        }

        WHEN("the sizes are different") {
            int64_t a = convert_to_integer<int64_t>(2, 31);
            int32_t b = convert_to_integer<int32_t>(0.25, 31);

            auto result = fixed_point_multiply<int32_t>(a, b, 31);
            THEN("the result should be 0.4") {
                int32_t expected = convert_to_integer<int32_t>(0.5, 31);
                REQUIRE(result == expected);
            }
        }

        WHEN("we have a small value") {
            int16_t a = convert_to_integer<int16_t>(2, 7);
            int16_t b = convert_to_integer<int16_t>(0.25, 7);

            auto result = fixed_point_multiply<int16_t>(a, b, 7);
            THEN("the result should be 0.4") {
                int32_t expected = convert_to_integer<int16_t>(0.5, 7);
                REQUIRE(result == expected);
            }
        }
    }
}
