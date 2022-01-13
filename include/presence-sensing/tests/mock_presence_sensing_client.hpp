#pragma once

#include <vector>

#include "presence-sensing/core/tasks/presence_sensing_driver_task.hpp"

namespace test_mocks {

struct MockHeadQueueClient {
    void send_presence_sensing_driver_queue(
        const presence_sensing_driver_task::TaskMessage& m) {
        messages.push_back(m);
    }
    std::vector<presence_sensing_driver_task::TaskMessage> messages{};
};
}  // namespace test_mocks