#pragma once

#include "eeprom/core/update_data_rev_task.hpp"

static constexpr uint16_t AXIS_DISTANCE_KEY = 0;
static constexpr uint16_t ERROR_COUNT_KEY = 1;

extern const eeprom::data_rev_task::DataTableUpdateMessage data_table_rev1;
extern const eeprom::data_rev_task::DataTableUpdateMessage data_table_rev2;

extern const std::array<eeprom::data_rev_task::DataTableUpdateMessage, 2>
    table_updater;