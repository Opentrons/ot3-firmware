#pragma once

#include <variant>
#pragma GCC diagnostic push
// NOLINTNEXTLINE(clang-diagnostic-unknown-warning-option)
#pragma GCC diagnostic ignored "-Wvolatile"
#include "common/firmware/adc.h"
#pragma GCC diagnostic pop
#include "common/core/freertos_message_queue.hpp"
#include "common/core/freertos_task.hpp"
#include "presence_sensor_driver.hpp"
#include "presence_sensor_messages.hpp"
#include "adc.hpp"


namespace presence_sensor_class {
template <adc::has_get_reading ADCDriver>
struct PresenceSensor {
    using GenericQueue = freertos_message_queue::FreeRTOSMessageQueue<presence_sensor_messages::Reading>;
    using CompleteQueue = freertos_message_queue::FreeRTOSMessageQueue<presence_sensor_messages::Ack>;
    PresenceSensor(ADCDriver& adc, GenericQueue& queue,
                   CompleteQueue& completed_queue)
        : pending_reading_queue(queue),
          completed_reading_queue(completed_queue),
          driver(adc) {}
    GenericQueue& pending_reading_queue;
    CompleteQueue& completed_reading_queue;
    presence_sensor_driver::PresenceSensorDriver<ADCDriver> driver;
    PresenceSensor(const PresenceSensor&) = delete;
    auto operator=(const PresenceSensor&) -> PresenceSensor& = delete;
    PresenceSensor(PresenceSensor&&) = delete;
    auto operator=(PresenceSensor&&) -> PresenceSensor&& = delete;
    ~PresenceSensor() = default;
};

}  // namespace presence_sensor_class