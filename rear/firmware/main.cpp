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
#include "common/firmware/gpio.hpp"
#include "common/firmware/iwdg.hpp"
#include "common/firmware/utility_gpio.h"
#include "i2c/firmware/i2c_comms.hpp"
#include "rear/core/tasks.hpp"
#include "rear/firmware/i2c_setup.h"


static auto iWatchdog = iwdg::IndependentWatchDog{};

 
/**
 * I2C handles
 */
static auto i2c_comms3 = i2c::hardware::I2C();
static auto i2c_handles = I2CHandlerStruct{};

class EEPromHardwareInterface
    : public eeprom::hardware_iface::EEPromHardwareIface {
  public:
    EEPromHardwareInterface()
        : eeprom::hardware_iface::EEPromHardwareIface(
              eeprom::hardware_iface::EEPromChipType::ST_M24128_DF) {}
    void set_write_protect(bool enable) final {
        if (enable) {
            disable_eeprom_write();
        } else {
            enable_eeprom_write();
        }
    }
};
static auto eeprom_hw_iface = EEPromHardwareInterface();


auto main() -> int {
    HardwareInit();
    RCC_Peripheral_Clock_Select();
    utility_gpio_init();

    i2c_setup(&i2c_handles);
    i2c_comms3.set_handle(i2c_handles.i2c3);

    rear_tasks::start_tasks(i2c_comms3, eeprom_hw_iface);

    iWatchdog.start(6);

    vTaskStartScheduler();
}
