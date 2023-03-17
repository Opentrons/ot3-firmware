#include "common/core/freertos_message_queue.hpp"

#include "rear-panel/core/tasks/light_control_task.hpp"
#include "rear-panel/firmware/led_hardware.h"

class LightControlHardware : public light_control_task::LightControlInterface {
    auto set_led_power(uint8_t id, uint32_t duty_cycle) -> void {
        led_hw_update_pwm(duty_cycle, static_cast<LED_DEVICE>(id));
    } 
};

static auto hardware = LightControlHardware();

static auto light_queue = freertos_message_queue::FreeRTOSMessageQueue<light_control_task::TaskMessage>("LightQueue");

auto task = light_control_task::LightControlTask(light_queue, hardware);