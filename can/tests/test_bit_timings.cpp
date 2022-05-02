#include "can/core/bit_timings.hpp"
#include "catch2/catch.hpp"

SCENARIO("bit timing properties") {
    WHEN("testing standard G4 properties") {
        auto timings =
            can::bit_timings::BitTimings<85000000, 50, 250000, 882>{};
        THEN("the time quantum is 47ns") {
            REQUIRE(timings.actual_time_quantum == 47);
        }
        THEN("the bitrate is 250kbaud") {
            REQUIRE(timings.actual_bitrate == 250312);
        }
        THEN("the segment 1 block is 73 quanta") {
            REQUIRE(timings.segment_1_quanta == 73);
        }
        THEN("the segment 2 block is 11 quantua") {
            REQUIRE(timings.segment_2_quanta == 11);
        }
        THEN("the sync jump width is 1 quantum") {
            REQUIRE(timings.max_sync_jump_width == 1);
        }
        THEN("the segment blocks add up to the right time") {
            REQUIRE(static_cast<uint32_t>(
                        static_cast<double>(can::bit_timings::NS_PER_S) /
                        static_cast<double>((timings.segment_1_quanta +
                                             timings.segment_2_quanta + 1) *
                                            timings.actual_time_quantum)) ==
                    timings.actual_bitrate);
        }
    }
    WHEN("testing standard L5 properties") {
        auto timings =
            can::bit_timings::BitTimings<110000000, 50, 250000, 882>{};
        THEN("the time quantum is 50ns") {
            REQUIRE(timings.actual_time_quantum == 45);
        }
        THEN("the bitrate is 250kbaud") {
            REQUIRE(timings.actual_bitrate == 252525);
        }
        THEN("the segment 1 block is 75 quanta") {
            REQUIRE(timings.segment_1_quanta == 76);
        }
        THEN("the segment 2 block is 11 quanta") {
            REQUIRE(timings.segment_2_quanta == 11);
        }
        THEN("the sync jump width is 1 quantum") {
            REQUIRE(timings.max_sync_jump_width == 1);
        }
        THEN("the segment blocks add to the right time") {
            REQUIRE(static_cast<uint32_t>(
                        static_cast<double>(can::bit_timings::NS_PER_S) /
                        static_cast<double>((timings.segment_1_quanta +
                                             timings.segment_2_quanta + 1) *
                                            timings.actual_time_quantum)) ==
                    timings.actual_bitrate);
        }
    }
}
