#pragma once

#include "eeprom/core/update_data_rev_task.hpp"

static constexpr uint16_t PLUNGER_MOTOR_STEP_KEY = 0;
static constexpr uint16_t GEAR_LEFT_MOTOR_KEY = 1;
static constexpr uint16_t GEAR_RIGHT_MOTOR_KEY = 2;
static constexpr uint16_t P_96_ERROR_COUNT_KEY = 3;
static constexpr uint16_t P_SM_ERROR_COUNT_KEY = 1;
static constexpr uint16_t L_ERROR_COUNT_KEY = 4;
static constexpr uint16_t R_ERROR_COUNT_KEY = 5;

extern const eeprom::data_rev_task::DataTableUpdateMessage
    data_table_rev1_sing_mult;
extern const eeprom::data_rev_task::DataTableUpdateMessage
    data_table_rev2_sing_mult;
extern const eeprom::data_rev_task::DataTableUpdateMessage data_table_rev1_96ch;
extern const eeprom::data_rev_task::DataTableUpdateMessage data_table_rev2_96ch;

extern const std::array<eeprom::data_rev_task::DataTableUpdateMessage, 2>
    table_updater;