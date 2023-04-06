#pragma once

#include "motor-control/core/tasks/usage_storage_task.hpp"
#include "eeprom/core/update_data_rev_task.hpp"

static constexpr eeprom::types::address Z_MOTOR_DIST_KEY = 0x0000;
static constexpr eeprom::types::address G_MOTOR_DIST_KEY = 0x0001;

static const eeprom::data_rev_task::DataTableUpdateMessage data_table_rev1{
                .data_rev = 1,
                .data_table = {std::make_pair(Z_MOTOR_DIST_KEY, usage_storage_task::distance_data_usage_len),
                               std::make_pair(G_MOTOR_DIST_KEY, usage_storage_task::distance_data_usage_len)}};

static const std::vector<eeprom::data_rev_task::DataTableUpdateMessage> table_updater = {
    // anytime there is an update to the data table add a message to this vector with the new key/length pairs
    data_table_rev1
};

// TODO Define keys for gripper motor