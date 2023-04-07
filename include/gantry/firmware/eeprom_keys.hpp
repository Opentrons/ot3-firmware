#pragma once

#include "eeprom/core/update_data_rev_task.hpp"
#include "motor-control/core/tasks/usage_storage_task.hpp"

static constexpr uint16_t AXIS_DISTANCE_KEY = 0;

static const eeprom::data_rev_task::DataTableUpdateMessage data_table_rev1{
    .data_rev = 1,
    .data_table = {std::make_pair(
        AXIS_DISTANCE_KEY, usage_storage_task::distance_data_usage_len)}};

static const std::vector<eeprom::data_rev_task::DataTableUpdateMessage>
    table_updater = {
        // anytime there is an update to the data table add a message to this
        // vector with the new key/length pairs
        data_table_rev1};