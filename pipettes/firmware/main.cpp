// clang-format off
#include "FreeRTOS.h"
#include "system_stm32l5xx.h"
#include "task.h"

// clang-format on

#include "can/core/ids.hpp"
#include "can/firmware/hal_can_bus.hpp"
#include "common/core/app_update.h"
#include "common/firmware/clocking.h"
#include "common/firmware/errors.h"
#include "common/firmware/i2c_comms.hpp"
#include "common/firmware/iwdg.hpp"
#include "common/firmware/spi_comms.hpp"
#include "common/firmware/utility_gpio.h"
#include "motor-control/core/linear_motion_system.hpp"
#include "motor-control/core/motor.hpp"
#include "motor-control/core/motor_driver_config.hpp"
#include "motor-control/core/motor_interrupt_handler.hpp"
#include "motor-control/core/motor_messages.hpp"
#include "motor-control/core/tmc2130_registers.hpp"
#include "motor-control/firmware/motor_hardware.hpp"
#include "mount_detection.hpp"
#include "pipettes/core/configs.hpp"
#include "pipettes/core/pipette_type.h"
#include "pipettes/core/tasks.hpp"

#pragma GCC diagnostic push
// NOLINTNEXTLINE(clang-diagnostic-unknown-warning-option)
#pragma GCC diagnostic ignored "-Wvolatile"
#include "motor_hardware.h"
#include "pipettes/firmware/i2c_setup.h"
#pragma GCC diagnostic pop

static auto PIPETTE_TYPE = get_pipette_type();

static auto iWatchdog = iwdg::IndependentWatchDog{};

static auto can_bus_1 = hal_can_bus::HalCanBus(can_get_device_handle());

static freertos_message_queue::FreeRTOSMessageQueue<motor_messages::Move>
    motor_queue("Motor Queue");

spi::SPI_interface SPI_intf = {
    .SPI_handle = &hspi2,
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
    .GPIO_handle = GPIOC,
    .pin = GPIO_PIN_6,
};
static spi::Spi spi_comms(SPI_intf);

static auto i2c_comms3 = i2c::I2C();
static I2CHandlerStruct i2chandler_struct{};

struct motion_controller::HardwareConfig plunger_pins {
    .direction =
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
            .port = GPIOC,
            .pin = GPIO_PIN_3,
            .active_setting = GPIO_PIN_SET},
    .step =
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
            .port = GPIOC,
            .pin = GPIO_PIN_7,
            .active_setting = GPIO_PIN_SET},
    .enable =
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
            .port = GPIOC,
            .pin = GPIO_PIN_8,
            .active_setting = GPIO_PIN_SET},
    .limit_switch =
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
            .port = GPIOC,
            .pin = GPIO_PIN_2,
            .active_setting = GPIO_PIN_SET},
    .led =
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
            .port = GPIOA,
            .pin = GPIO_PIN_8,
            .active_setting = GPIO_PIN_RESET},
    .sync_in = {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
        .port = GPIOB,
        .pin = GPIO_PIN_8,
        .active_setting = GPIO_PIN_RESET},
}
}
;

static motor_hardware::MotorHardware plunger_hw(plunger_pins, &htim7);
static motor_handler::MotorInterruptHandler plunger_interrupt(
    motor_queue, pipettes_tasks::get_queues(), plunger_hw);

/**
 * TODO: This motor class is only used in motor handler and should be
 * instantiated inside of the MotorHandler class. However, some refactors
 * should be made to avoid a pretty gross template signature.
 */

static motor_class::Motor pipette_motor{
    spi_comms,
    configs::linear_motion_sys_config_by_axis(PIPETTE_TYPE),
    plunger_hw,
    motor_messages::MotionConstraints{.min_velocity = 1,
                                      .max_velocity = 2,
                                      .min_acceleration = 1,
                                      .max_acceleration = 2},
    configs::driver_config_by_axis(PIPETTE_TYPE),
    motor_queue};

extern "C" void plunger_callback() { plunger_interrupt.run_interrupt(); }

auto main() -> int {
    HardwareInit();
    RCC_Peripheral_Clock_Select();
    MX_ICACHE_Init();
    utility_gpio_init();
    adc_init();
    auto id = pipette_mounts::detect_id();

    i2c_setup(&i2chandler_struct, PIPETTE_TYPE);
    i2c_comms3.set_handle(i2chandler_struct.i2c3);

    if (initialize_spi() != HAL_OK) {
        Error_Handler();
    }

    app_update_clear_flags();

    initialize_timer(plunger_callback);

    can_start();

    pipettes_tasks::start_tasks(can_bus_1, pipette_motor.motion_controller,
                                pipette_motor.driver, i2c_comms3, id);

    iWatchdog.start(6);

    vTaskStartScheduler();
}
