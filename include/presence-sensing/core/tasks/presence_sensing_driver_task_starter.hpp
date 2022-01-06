#pragma once

#include "common/core/freertos_message_queue.hpp"
#include "common/core/freertos_task.hpp"
#include "common/core/adc.hpp"
#include "common/firmware/adc_comms.hpp"
#include "presence-sensing/core/tasks/presence_sensing_driver_task.hpp"
#include "presence-sensing/core/presence_sensing_driver.hpp"

namespace presence_sensing_driver_task_starter {

template <uint32_t StackDepth,
          message_writer_task::TaskClient CanClient,
          adc::has_get_reading ADCDriver>
class TaskStarter {
  public:
    using PresenceSensingDriverTaskType =
        presence_sensing_driver_task::PresenceSensingDriverTask<
            freertos_message_queue::FreeRTOSMessageQueue, CanClient, adc::ADC>;
    using QueueType = freertos_message_queue::FreeRTOSMessageQueue<
        presence_sensing_driver_task::TaskMessage>;
    using TaskType =
        freertos_task::FreeRTOSTask<StackDepth, PresenceSensingDriverTaskType,
                                     CanClient>;

    TaskStarter() : task_entry{queue}, task{task_entry} {}
    TaskStarter(const TaskStarter& c) = delete;
    TaskStarter(const TaskStarter&& c) = delete;
    auto operator=(const TaskStarter& c) = delete;
    auto operator=(const TaskStarter&& c) = delete;
    ~TaskStarter() = default;

    auto start(uint32_t priority, presence_sensing_driver::PresenceSensingDriver<ADCDriver>& presence_sensing_driver,
               CanClient& can_client) -> PresenceSensingDriverTaskType& {
        task.start(priority, "presence-sensing", &presence_sensing_driver, &can_client);
        return task_entry;
    }

  private:
    QueueType queue{};
    PresenceSensingDriverTaskType task_entry;
    TaskType task;
};
}  // namespace motion_controller_task_starter