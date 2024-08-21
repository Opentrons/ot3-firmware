#include <array>
#include <cstdio>
#include <cstring>

// clang-format off
#include "FreeRTOS.h"
#include "task.h"
#include "system_stm32g4xx.h"
#include "gantry/firmware/i2c_setup.h"
// clang-format on

#include "common/core/app_update.h"
#include "common/firmware/clocking.h"
#include "common/firmware/utility_gpio.h"
#include "gantry/core/interfaces_rev1.hpp"
#include "gantry/core/tasks_rev1.hpp"
#include "i2c/firmware/i2c_comms.hpp"

static interfaces::diag0_handler call_diag0_handler = nullptr;

static auto i2c_comms2 = i2c::hardware::I2C();
static auto i2c_handles = I2CHandlerStruct{};

static constexpr auto eeprom_chip =
    eeprom::hardware_iface::EEPromChipType::ST_M24128_BF;

class EEPromHardwareInterface
    : public eeprom::hardware_iface::EEPromHardwareIface {
  public:
    EEPromHardwareInterface()
        : eeprom::hardware_iface::EEPromHardwareIface(eeprom_chip) {}
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

    app_update_clear_flags();

    interfaces::initialize(&call_diag0_handler);
    i2c_setup(&i2c_handles);
    i2c_comms2.set_handle(i2c_handles.i2c2);

    call_diag0_handler = gantry::tasks::start_tasks(
        interfaces::get_can_bus(), interfaces::get_motor().motion_controller,
        interfaces::get_spi(), interfaces::get_driver_config(),
        interfaces::get_motor_hardware_task(), i2c_comms2, eeprom_hw_iface);

    vTaskStartScheduler();
}
