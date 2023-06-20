#include "catch2/catch.hpp"
#include "ot_utils/core/filters/ema.hpp"

using namespace ot_utils;

template<class uint_t>
float fixed_point_to_float(uint_t data, int to_radix) {
    return (1.0 * static_cast<float>(data)) / static_cast<float>(1 << to_radix);
}

SCENARIO("Test for EMA filtering") {

	const int radix = 7;
	const int alpha_divisor = 2;
	filters::ExponentialMovingAverage<uint16_t, alpha_divisor, radix> filter{};
    GIVEN("An array of inputs") {
		std::array<float, 5> inputs{1, 6, 3, 4, 2};
		std::array<float, 5> outputs{0.5, 3.25, 3.125, 3.5625, 2.78125};

		THEN("The result of compute is the expected value") {
			for (int i = 0; i < static_cast<int>(inputs.size()) - 1; i++) {
				auto fixed_input = fixed_point::convert_to_fixed_point<uint16_t>(inputs[i], radix);
				auto result = filter.compute(fixed_input);

				auto floating_result = fixed_point_to_float<uint16_t>(result, radix);
				REQUIRE(floating_result == outputs[i]);

			}
		}

	}
}
