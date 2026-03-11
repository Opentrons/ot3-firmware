#include <cstdint>
#include <cstring>
#include <iostream>

#include "eeprom/core/dev_data.hpp"
#include "eeprom/core/hardware_iface.hpp"

extern "C" {
void vTaskDelay(const int x) { std::ignore = x; }
void vTaskDelete(void* x) { std::ignore = x; }
}

#include "catch2/catch.hpp"
#include "eeprom/core/types.hpp"
#include "eeprom/core/update_data_rev_task.hpp"

using namespace eeprom;

struct MockEEPromTaskClient {
    void send_eeprom_queue(const task::TaskMessage& m) {
        messages.push_back(m);

        if (auto* config_message =
                std::get_if<eeprom::message::ConfigRequestMessage>(&m)) {
            const eeprom::message::ConfigRequestCallback& callback =
                config_message->callback;
            void* callback_param = config_message->callback_param;

            eeprom::message::ConfigResponseMessage response =
                eeprom::message::ConfigResponseMessage{};

            response.chip =
                eeprom::hardware_iface::EEPromChipType::ST_M24128_DR;
            response.addr_bytes = sizeof(uint8_t);
            response.mem_size = static_cast<types::address>(
                hardware_iface::EEpromMemorySize::ST_16_KBYTE);
            response.default_byte_value = 0xFF;

            callback(response, callback_param);
        }
    }
    std::vector<task::TaskMessage> messages{};
};

SCENARIO("Sending migrate data message") {
    // migrate data message
    const uint16_t data_revision = 1;
    std::vector<std::pair<types::address, types::data_length>> data = {
        {0, 8},
        {1, 8},
        {2, 8},
    };

    const data_rev_task::MigrateDataMessage mock_data_message{
        .data_rev = data_revision, .data_table = data};

    // Test handler
    auto eeprom_client = MockEEPromTaskClient{};
    auto tail_accessor = eeprom::dev_data::DevDataTailAccessor{eeprom_client};
    auto data_rev_handler =
        data_rev_task::UpdateDataRevHandler{eeprom_client, tail_accessor};

    GIVEN("Finding boundary of old data") {
        types::address eeprom_length = static_cast<types::address>(
            hardware_iface::EEpromMemorySize::ST_16_KBYTE);

        data_rev_handler.handle_message(mock_data_message);

        types::address end_address = eeprom_length - 64;

        THEN("address should have updated to reflect the new location") {
            REQUIRE(addresses::DataAddressWrapper::get_ot_library_begin() ==
                    192);
            REQUIRE(
                eeprom::addresses::DataAddressWrapper::get_ot_library_end() ==
                end_address);
        }

        const std::vector<std::pair<types::address, types::data_length>>
            additional = {{4, 8}, {5, 8}, {6, 8}, {7, 8}, {8, 8}, {9, 8}};

        data.insert(data.end(), additional.begin(), additional.end());

        const data_rev_task::MigrateDataMessage dummy_data_message{
            .data_rev = data_revision + 1, .data_table = data};

        data_rev_handler.handle_message(dummy_data_message);

        // The lock should hold, the value embedded within shouldn't change
        // the value
        THEN("The data address lock should hold") {
            REQUIRE(addresses::ot_library_end == end_address);
        }
    }
}
