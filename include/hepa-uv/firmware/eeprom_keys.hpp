#pragma once

#include "eeprom/core/update_data_rev_task.hpp"

static constexpr eeprom::types::address Z_MOTOR_DIST_KEY = 0x0000;
static constexpr eeprom::types::address G_MOTOR_DIST_KEY = 0x0001;
static constexpr eeprom::types::address G_MOTOR_FORCE_TIME_KEY = 0x0002;
static constexpr eeprom::types::address Z_ERROR_COUNT_KEY = 0x0003;
static constexpr eeprom::types::address G_ERROR_COUNT_KEY = 0x0004;

extern const eeprom::data_rev_task::DataTableUpdateMessage data_table_rev1;
extern const eeprom::data_rev_task::DataTableUpdateMessage data_table_rev2;
extern const eeprom::data_rev_task::DataTableUpdateMessage data_table_rev3;

extern const std::vector<eeprom::data_rev_task::DataTableUpdateMessage>
    table_updater;
