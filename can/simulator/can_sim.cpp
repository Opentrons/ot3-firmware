#include "simulator/can_bus.hpp"

void SimulatingCanBus::add_exact_filter(uint32_t match1, uint32_t match2) {
    auto func = [](auto a, auto b, auto val) -> bool {
        return val == a or val == b;
    };
    this->filters.push_back(Filter(match1, match2, func));
}

void SimulatingCanBus::add_mask_filter(uint32_t filter, uint32_t mask) {
    auto func = [](auto a, auto b, auto val) -> bool { return (val & b) == a; };
    this->filters.push_back(Filter(filter, mask, func));
}

void SimulatingCanBus::add_range_filter(uint32_t minimum, uint32_t maximum) {
    auto func = [](auto a, auto b, auto val) -> bool {
        return val >= a and val <= b;
    };
    this->filters.push_back(Filter(minimum, maximum, func));
}
