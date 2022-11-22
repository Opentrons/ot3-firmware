#pragma once
#include "eeprom/core/accessor.hpp"

using namespace eeprom;

struct MockListener : accessor::ReadListener {
    void read_complete(uint32_t message_index) {
        call_count++;
        std::ignore = message_index;
    }
    int call_count{0};
};
