#include "can/core/bit_timings.hpp"
#include "catch2/catch.hpp"

using can::bit_timings::KHZ;
using can::bit_timings::MHZ;

SCENARIO("bit timing properties") {
    WHEN("testing standard G4 properties") {
        auto timings =
            can::bit_timings::BitTimings<170 * MHZ, 100, 500 * KHZ, 800>{};
        THEN("the clock divider is 17") {
            REQUIRE(timings.clock_divider == 17);
        }
        THEN("the time quantum is 100ns") {
            REQUIRE(timings.actual_time_quantum == 100);
        }
        THEN("there are 20 time quanta in a bit for a 200ns bit time") {
            REQUIRE(timings.total_time_quanta == 20);
        }
        THEN("the bitrate is 500kbaud") {
            REQUIRE(timings.actual_bitrate == 500 * KHZ);
        }
        THEN("the segment 1 block is 15 quanta") {
            REQUIRE(timings.segment_1_quanta == 15);
        }
        THEN("the segment 2 block is 4 quanta") {
            REQUIRE(timings.segment_2_quanta == 4);
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
            can::bit_timings::BitTimings<110 * MHZ, 100, 500 * KHZ, 800>{};
        THEN("the clock divider is 11") {
            REQUIRE(timings.clock_divider == 11);
        }
        THEN("the time quantum is 100ns") {
            REQUIRE(timings.actual_time_quantum == 100);
        }
        THEN("the bitrate is 500kbaud") {
            REQUIRE(timings.actual_bitrate == 500 * KHZ);
        }
        THEN("there are 20 time quanta in a bit") {
            REQUIRE(timings.total_time_quanta == 20);
        }
        THEN("the segment 1 block is 15 quanta") {
            REQUIRE(timings.segment_1_quanta == 15);
        }
        THEN("the segment 2 block is 4 quanta") {
            REQUIRE(timings.segment_2_quanta == 4);
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
