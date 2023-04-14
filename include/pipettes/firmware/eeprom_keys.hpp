#pragma once

#include "eeprom/core/update_data_rev_task.hpp"

static constexpr uint16_t PLUNGER_MOTOR_STEP_KEY = 0;
static constexpr uint16_t GEAR_LEFT_MOTOR_KEY = 1;
static constexpr uint16_t GEAR_RIGHT_MOTOR_KEY = 2;

extern const eeprom::data_rev_task::DataTableUpdateMessage
    data_table_rev1_sing_mult;
extern const eeprom::data_rev_task::DataTableUpdateMessage data_table_rev1_96ch;

extern const std::vector<eeprom::data_rev_task::DataTableUpdateMessage>
    table_updater;