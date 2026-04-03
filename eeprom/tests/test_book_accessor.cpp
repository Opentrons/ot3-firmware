#include <cstdint>
#include <tuple>

extern "C" {
void vTaskDelay(const int x) { std::ignore = x; }
void vTaskDelete(void* x) { std::ignore = x; }
}

#include "catch2/catch.hpp"
#include "eeprom/core/book_accessor.hpp"
#include "eeprom/core/messages.hpp"
#include "eeprom/core/task.hpp"
#include "eeprom/core/types.hpp"

using namespace eeprom;

struct MockEEPromTaskClient {
    uint16_t read_counter = 0;

    void send_eeprom_message(const task::TaskMessage& m) {
        if (const auto* read_message =
                std::get_if<eeprom::message::ReadEepromMessage>(&m)) {
            // switch (read_counter) {
            //   // correct data to be read
            //   case 0:
            //
        }
    }
};
