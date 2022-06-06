#include <array>

#include "can/core/can_bus.hpp"
#include "catch2/catch.hpp"

using namespace can::bus;

SCENARIO("to_canfd_length works") {
    GIVEN("an array of integer lengths") {
        auto arr = std::array{
            std::pair{0, CanFDMessageLength::l0},
            std::pair{1, CanFDMessageLength::l1},
            std::pair{2, CanFDMessageLength::l2},
            std::pair{3, CanFDMessageLength::l3},
            std::pair{4, CanFDMessageLength::l4},
            std::pair{5, CanFDMessageLength::l5},
            std::pair{6, CanFDMessageLength::l6},
            std::pair{7, CanFDMessageLength::l7},
            std::pair{8, CanFDMessageLength::l8},
            std::pair{10, CanFDMessageLength::l12},
            std::pair{12, CanFDMessageLength::l12},
            std::pair{14, CanFDMessageLength::l16},
            std::pair{16, CanFDMessageLength::l16},
            std::pair{18, CanFDMessageLength::l20},
            std::pair{20, CanFDMessageLength::l20},
            std::pair{22, CanFDMessageLength::l24},
            std::pair{24, CanFDMessageLength::l24},
            std::pair{28, CanFDMessageLength::l32},
            std::pair{32, CanFDMessageLength::l32},
            std::pair{44, CanFDMessageLength::l48},
            std::pair{48, CanFDMessageLength::l48},
            std::pair{50, CanFDMessageLength::l64},
            std::pair{64, CanFDMessageLength::l64},
            std::pair{164, CanFDMessageLength::l64},
        };
        WHEN("converted to an fdcan length") {
            THEN("it is a CanFDMessageLength value") {
                for (auto v : arr) {
                    REQUIRE(to_canfd_length(v.first) == v.second);
                }
            }
        }
    }
}