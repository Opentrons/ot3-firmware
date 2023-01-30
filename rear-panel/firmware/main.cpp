#include <array>

// clang-format off
#include "FreeRTOS.h"
#include "task.h"
#include "system_stm32g4xx.h"
// clang-format on
#pragma GCC diagnostic push
// NOLINTNEXTLINE(clang-diagnostic-unknown-warning-option)
#pragma GCC diagnostic ignored "-Wvolatile"
#include "stm32g4xx_hal.h"
#pragma GCC diagnostic pop

#include "common/firmware/clocking.h"
//#include "common/firmware/gpio.hpp"
#include "common/firmware/iwdg.hpp"
#include "common/firmware/utility_gpio.h"
//#include "i2c/firmware/i2c_comms.hpp"
#include "rear-panel/core/tasks.hpp"
//#include "rear-panel/firmware/i2c_setup.h"
#include "rear-panel/firmware/freertos_comms_task.hpp"
#include "rear-panel/firmware/led_hardware.h"

static auto iWatchdog = iwdg::IndependentWatchDog{};
/**
 * I2C handles
 */
/*

static auto i2c_comms3 = i2c::hardware::I2C();
static auto i2c_handles = I2CHandlerStruct{};

class EEPromHardwareInterface
    : public eeprom::hardware_iface::EEPromHardwareIface {
  public:
    EEPromHardwareInterface()
        : eeprom::hardware_iface::EEPromHardwareIface(
              eeprom::hardware_iface::EEPromChipType::ST_M24128_BF) {}
    void set_write_protect(bool enable) final {
        if (enable) {
            disable_eeprom_write();
        } else {
            enable_eeprom_write();
        }
    }
};
static auto eeprom_hw_iface = EEPromHardwareInterface();
*/

static constexpr uint32_t TASK_STACK_DEPTH = 512;

class LED_BLINK_TASK {
  public:
    LED_BLINK_TASK(const char* task_name) : task_name{task_name} {}
    LED_BLINK_TASK(const LED_BLINK_TASK& c) = delete;
    LED_BLINK_TASK(const LED_BLINK_TASK&& c) = delete;
    auto operator=(const LED_BLINK_TASK& c) = delete;
    auto operator=(const LED_BLINK_TASK&& c) = delete;
    ~LED_BLINK_TASK() = default;

    void start_task() {
        xTaskCreateStatic(this->task_function, task_name, TASK_STACK_DEPTH,
                          (void*)1, 6, backing.data(), &static_task);
    }

    /**
     * Task entry point.
     */
    [[noreturn]] static void task_function(void* unused) {
        std::ignore = unused;
        for (;;) {
            vTaskDelay(500);
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_RESET);
            vTaskDelay(500);
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_SET);
        }
    }

  private:
    const char* task_name;
    std::array<StackType_t, TASK_STACK_DEPTH> backing{};
    StaticTask_t static_task{};
};

static auto lbt = LED_BLINK_TASK{"Blinkenlights"};

auto main() -> int {
    HardwareInit();
    RCC_Peripheral_Clock_Select();
    utility_gpio_init();

    // initialize_leds();

    // i2c_setup(&i2c_handles);
    // i2c_comms3.set_handle(i2c_handlines.i2c3);

    // rear_tasks::start_tasks(i2c_comms3, eeprom_hw_iface);
    rear_panel_tasks::start_tasks();
    lbt.start_task();
    iWatchdog.start(6);

    vTaskStartScheduler();
}
