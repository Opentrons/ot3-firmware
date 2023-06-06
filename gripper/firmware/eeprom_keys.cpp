// clang-format off
#include "FreeRTOS.h"
#include "task.h"
// clang-format on

#include "gripper/firmware/eeprom_keys.hpp"

#include "motor-control/core/tasks/usage_storage_task.hpp"

const eeprom::data_rev_task::DataTableUpdateMessage data_table_rev1{
    .data_rev = 1,
    .len = 2,
    .data_table = {
        std::make_pair(Z_MOTOR_DIST_KEY,
                       usage_storage_task::distance_data_usage_len),
        std::make_pair(G_MOTOR_DIST_KEY,
                       usage_storage_task::distance_data_usage_len)}};

const eeprom::data_rev_task::DataTableUpdateMessage data_table_rev2{
    .data_rev = 2,
    .len = 1,
    .data_table = {
        std::make_pair(G_MOTOR_FORCE_TIME_KEY,
                       usage_storage_task::force_time_data_usage_len)}};
const eeprom::data_rev_task::DataTableUpdateMessage data_table_rev3{
    .data_rev = 3,
    .len = 2,
    .data_table = {std::make_pair(Z_ERROR_COUNT_KEY,
                                  usage_storage_task::error_count_usage_len),
                   std::make_pair(G_ERROR_COUNT_KEY,
                                  usage_storage_task::error_count_usage_len)}};

const std::array<eeprom::data_rev_task::DataTableUpdateMessage, 3>  table_updater =
    {
        // anytime there is an update to the data table add a message to this
        // vector with the new key/length pairs
        data_table_rev1, data_table_rev2, data_table_rev3};
