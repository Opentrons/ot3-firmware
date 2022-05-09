#include "can/core/bit_timings.hpp"
#include "catch2/catch.hpp"

using can::bit_timings::KHZ;
using can::bit_timings::MHZ;

SCENARIO("bit timing properties") {
    WHEN("testing standard G4 properties") {
        auto timings =
            can::bit_timings::BitTimings<170 * MHZ, 100, 250 * KHZ, 800>{};
        THEN("the clock divider is 4") { REQUIRE(timings.clock_divider == 4); }
        THEN("the time quantum is 47ns") {
            REQUIRE(timings.actual_time_quantum == 47);
        }
        THEN("there are 85 time quanta in a bit") {
            REQUIRE(timings.total_time_quanta == 85);
        }
        THEN("the bitrate is 250kbaud") {
            REQUIRE(timings.actual_bitrate == 250312);
        }
        THEN("the segment 1 block is 73 quanta") {
            REQUIRE(timings.segment_1_quanta == 73);
        }
        THEN("the segment 2 block is 11 quanta") {
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
            can::bit_timings::BitTimings<110 * MHZ, 100, 250 * KHZ, 800>{};
        THEN("the clock divider is 50") { REQUIRE(timings.clock_divider == 5); }
        THEN("the time quantum is 45ns") {
            REQUIRE(timings.actual_time_quantum == 45);
        }
        THEN("the bitrate is 250kbaud") {
            REQUIRE(timings.actual_bitrate == 252525);
        }
        THEN("there are 88 time quanta in a bit") {
            REQUIRE(timings.total_time_quanta == 88);
        }
        THEN("the segment 1 block is 76 quanta") {
            REQUIRE(timings.segment_1_quanta == 76);
        }
        THEN("the segment 2 block is 11 quantum") {
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
