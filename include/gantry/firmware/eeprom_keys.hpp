#pragma once

#include "eeprom/core/update_data_rev_task.hpp"

static constexpr uint16_t AXIS_DISTANCE_KEY = 0;

extern const eeprom::data_rev_task::DataTableUpdateMessage data_table_rev1;

extern const std::vector<eeprom::data_rev_task::DataTableUpdateMessage>
    table_updater;