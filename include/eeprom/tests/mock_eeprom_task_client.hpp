#pragma once

using namespace eeprom;

struct MockEEPromTaskClient {
    void send_eeprom_queue(const task::TaskMessage& m) {
        messages.push_back(m);
    }
    std::vector<task::TaskMessage> messages{};
};
