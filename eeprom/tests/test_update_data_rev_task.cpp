#include <cstring>
#include "../../include/eeprom/core/dev_data.hpp"

extern "C" {
    void vTaskDelay(const int x) {std::ignore = x;}
    void vTaskDelete(void* x) {std::ignore = x;}
}

#include "../../include/eeprom/core/types.hpp"
#include "../../include/eeprom/core/update_data_rev_task.hpp"
#include "../../include/eeprom/tests/mock_eeprom_task_client.hpp"
#include "../../stm32-tools/catch2/src/single_include/catch2/catch.hpp"

using namespace eeprom;

SCENARIO("Sending migrate data message") {
    // migrate data message
    const uint16_t data_revision = 1;
    std::vector<std::pair<types::address, types::data_length>> data = {
        {0, 8},
        {1, 8},
        {2, 8},
    };

    const data_rev_task::MigrateDataMessage mock_data_message{
        .data_rev = data_revision,
        .data_table = data
    };

    // Test handler
    auto eeprom_client = MockEEPromTaskClient{};
    auto tail_accessor = eeprom::dev_data::DevDataTailAccessor{eeprom_client};
    auto data_rev_handler = data_rev_task::UpdateDataRevHandler{
    eeprom_client, tail_accessor};

    GIVEN("Finding boundary of old data") {
        data_rev_handler.handle_message(mock_data_message);

        THEN("address should have updated to reflect the new location") {
            REQUIRE(addresses::ot_library_begin == addresses::data_address_begin);
            REQUIRE(addresses::ot_library_end == 64);
        }

        const std::optional<types::address> end_address = addresses::ot_library_end;

        const std::vector<std::pair<types::address, types::data_length>>  additional= {
            {4, 8},
            {5, 8},
            {6, 8},
            {7, 8},
            {8, 8},
            {9, 8}
        };

        data.insert(data.end(), additional.begin(), additional.end());

        const data_rev_task::MigrateDataMessage dummy_data_message{
        .data_rev = data_revision+1,
        .data_table = data
        };

        data_rev_handler.handle_message(dummy_data_message);

        // The lock should hold, the value embedded within shouldn't change
        // the value
        THEN("The data address lock should hold") {
            REQUIRE(addresses::ot_library_end == end_address);
        }
   }
}