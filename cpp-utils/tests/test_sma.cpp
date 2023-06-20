#include "catch2/catch.hpp"
#include "ot_utils/core/filters/sma.hpp"

using namespace ot_utils;

template<class uint_t>
float fixed_point_to_float(uint_t data, int to_radix) {
    return (1.0 * static_cast<float>(data)) / static_cast<float>(1 << to_radix);
}

SCENARIO("SMA filtering for various window sizes") {

	const int radix = 7;
	const int window_size = 3;
	filters::SimpleMovingAverage<int16_t, window_size, radix> filter{};
    GIVEN("An array of inputs") {
		std::array<float, 5> inputs{1, 6, 3, 4, 2};
		std::array<float, 5> outputs{1, 6, 3.33, 4.33, 3.00};

		THEN("The result of compute is the expected value") {
			for (int i = 0; i < static_cast<int>(inputs.size()) - 1; i++) {
				auto fixed_input = fixed_point::convert_to_fixed_point<int16_t>(inputs[i], radix);
				auto result = filter.compute(fixed_input);

				auto floating_result = fixed_point_to_float<int16_t>(result, radix);
				REQUIRE(floating_result == Approx(outputs[i]).epsilon(1e-1));

			}
		}
	}

	GIVEN("A large set of points that level out") {
		std::array<float, 12> inputs{1, 3, 3, 2, 2, 2.9, 2.4, 2.5, 2.8, 2.5, 2.8, 2.5};

		THEN("When we check if the data is stable, result is true") {
			for (int i = 0; i < static_cast<int>(inputs.size()) - 1; i++) {
				auto fixed_input = fixed_point::convert_to_fixed_point<int16_t>(inputs[i], radix);
				static_cast<void>(filter.compute(fixed_input));
			}
			REQUIRE(filter.stop_filter() == true);
		}

	}

	GIVEN("A large set of points that do not level out") {
		std::array<float, 12> inputs{1, 2, 2, 2, 2, 0, 1, 3, 1, 15, 70, 64};

		THEN("When we check if the data is stable, result is false") {
			for (int i = 0; i < static_cast<int>(inputs.size()) - 1; i++) {
				auto fixed_input = fixed_point::convert_to_fixed_point<int16_t>(inputs[i], radix);
				static_cast<void>(filter.compute(fixed_input));
			}
			REQUIRE(filter.stop_filter() == false);
		}

	}
}
