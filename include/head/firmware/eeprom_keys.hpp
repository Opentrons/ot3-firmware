#pragma once

#include "eeprom/core/update_data_rev_task.hpp"

static constexpr uint16_t L_MOTOR_DISTANCE_KEY = 0;
static constexpr uint16_t R_MOTOR_DISTANCE_KEY = 1;
static constexpr uint16_t L_ERROR_COUNT_KEY = 2;
static constexpr uint16_t R_ERROR_COUNT_KEY = 3;

extern const eeprom::data_rev_task::DataTableUpdateMessage data_table_rev1;
extern const eeprom::data_rev_task::DataTableUpdateMessage data_table_rev2;

extern const std::array<eeprom::data_rev_task::DataTableUpdateMessage, 2>
    table_updater;