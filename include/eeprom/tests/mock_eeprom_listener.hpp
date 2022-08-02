#pragma once
#include "eeprom/core/accessor.hpp"

using namespace eeprom;

struct MockListener : accessor::ReadListener {
    void read_complete() { call_count++; }
    int call_count{0};
};
