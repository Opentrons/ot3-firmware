#pragma once

#include <vector>

#include "motor-control/core/tasks/tmc_motor_driver_common.hpp"

namespace test_mocks {

struct MockMotorDriverClient {
    void send_motor_driver_queue(const tmc::tasks::TaskMessage& m) {
        messages.push_back(m);
    }

    std::vector<tmc::tasks::TaskMessage> messages{};
};

}  // namespace test_mocks