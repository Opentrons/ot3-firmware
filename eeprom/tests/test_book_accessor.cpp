#include "eeprom/core/dev_data.hpp"
#include "eeprom/core/hardware_iface.hpp"

extern "C" {
void vTaskDelay(const int x) { std::ignore = x; }
void vTaskDelete(void* x) { std::ignore = x; }
}

#include "catch2/catch.hpp"
#include "eeprom/core/messages.hpp"
#include "eeprom/core/task.hpp"
#include "eeprom/core/types.hpp"
#include "eeprom/core/update_data_rev_task.hpp"

using namespace eeprom;

struct MockEEPromTaskClient {
    uint16_t read_counter = 0;

    void send_eeprom_message(const task::TaskMesage& m) {
        if (const message::ReadEepromMessage read_message =
                std::get_if<message::ReadEepromMessage>(m)) {
        }
    }
}
