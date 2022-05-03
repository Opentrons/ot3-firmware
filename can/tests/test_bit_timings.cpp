#include "can/core/bit_timings.hpp"
#include "catch2/catch.hpp"

using can::bit_timings::KHZ;
using can::bit_timings::MHZ;

SCENARIO("bit timing properties") {
    WHEN("testing standard G4 properties") {
        auto timings =
            can::bit_timings::BitTimings<85 * MHZ, 240, 250 * KHZ, 883>{};
        THEN("the clock divider is 20") {
            REQUIRE(timings.clock_divider == 20);
        }
        THEN("the time quantum is 235ns") {
            REQUIRE(timings.actual_time_quantum == 235);
        }
        THEN("there are 17 time quanta in a bit") {
            REQUIRE(timings.total_time_quanta == 17);
        }
        THEN("the bitrate is 250kbaud") {
            REQUIRE(timings.actual_bitrate == 250312);
        }
        THEN("the segment 1 block is 73 quanta") {
            REQUIRE(timings.segment_1_quanta == 14);
        }
        THEN("the segment 2 block is 11 quanta") {
            REQUIRE(timings.segment_2_quanta == 2);
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
            can::bit_timings::BitTimings<110 * MHZ, 455, 275330, 875>{};
        THEN("the clock divider is 50") {
            REQUIRE(timings.clock_divider == 50);
        }
        THEN("the time quantum is 50ns") {
            REQUIRE(timings.actual_time_quantum == 454);
        }
        THEN("the bitrate is 250kbaud") {
            REQUIRE(timings.actual_bitrate == 275330);
        }
        THEN("there are 8 time quanta in a bit") {
            REQUIRE(timings.total_time_quanta == 8);
        }
        THEN("the segment 1 block is 6 quanta") {
            REQUIRE(timings.segment_1_quanta == 6);
        }
        THEN("the segment 2 block is 1 quantum") {
            REQUIRE(timings.segment_2_quanta == 1);
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
