// clang-format off
#include "FreeRTOS.h"
#include "system_stm32l5xx.h"
#include "task.h"

// clang-format on

#include "can/core/bit_timings.hpp"
#include "can/core/ids.hpp"
#include "can/firmware/hal_can_bus.hpp"
#include "common/core/app_update.h"
#include "common/firmware/clocking.h"
#include "common/firmware/errors.h"
#include "common/firmware/gpio.hpp"
#include "common/firmware/iwdg.hpp"
#include "common/firmware/utility_gpio.h"

#include "eeprom/core/hardware_iface.hpp"

// todo check if needed
#include "motor-control/core/linear_motion_system.hpp"

#include "motor-control/core/motor_messages.hpp"
#include "motor-control/core/stepper_motor/motor.hpp"
#include "motor-control/core/stepper_motor/motor_interrupt_handler.hpp"
#include "motor-control/firmware/stepper_motor/motor_hardware.hpp"
#include "mount_detection.hpp"
#include "pipettes/core/central_tasks.hpp"
#include "pipettes/core/configs.hpp"
#include "pipettes/core/gear_motor_tasks.hpp"
#include "pipettes/core/interfaces.hpp"
#include "pipettes/core/linear_motor_tasks.hpp"
#include "pipettes/core/peripheral_tasks.hpp"
#include "pipettes/core/pipette_type.h"
#include "pipettes/core/sensor_tasks.hpp"
#include "sensors/firmware/sensor_hardware.hpp"
#include "spi/firmware/spi_comms.hpp"
#include "i2c/firmware/i2c_comms.hpp"

#pragma GCC diagnostic push
// NOLINTNEXTLINE(clang-diagnostic-unknown-warning-option)
#pragma GCC diagnostic ignored "-Wvolatile"
#include "motor_encoder_hardware.h"
#include "motor_hardware.h"
#include "motor_timer_hardware.h"
#include "pipettes/firmware/i2c_setup.h"
#pragma GCC diagnostic pop

static auto PIPETTE_TYPE = get_pipette_type();

static auto iWatchdog = iwdg::IndependentWatchDog{};

static auto can_bus_1 = hal_can_bus::HalCanBus(can_get_device_handle());

static freertos_message_queue::FreeRTOSMessageQueue<motor_messages::Move>
    motor_queue("Motor Queue");

spi::hardware::SPI_interface SPI_intf = {.SPI_handle = &hspi2};

static spi::hardware::Spi spi_comms(SPI_intf);

static auto i2c_comms3 = i2c::hardware::I2C();
static auto i2c_comms1 = i2c::hardware::I2C();
static I2CHandlerStruct i2chandler_struct{};

class EEPromHardwareIface : public eeprom::hardware_iface::EEPromHardwareIface {
  public:
    void set_write_protect(bool enable) final {
        if (enable) {
            disable_eeprom();
        } else {
            enable_eeprom();
        }
    }
};
static auto eeprom_hardware_iface = EEPromHardwareIface();

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
    .led = {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
        .port = GPIOA,
        .pin = GPIO_PIN_8,
        .active_setting = GPIO_PIN_RESET},
};

static motor_hardware::MotorHardware plunger_hw(plunger_pins, &htim7, &htim2);
static motor_handler::MotorInterruptHandler plunger_interrupt(
    motor_queue, linear_motor_tasks::get_queues(), plunger_hw);

/**
 * TODO: This motor class is only used in motor handler and should be
 * instantiated inside of the MotorHandler class. However, some refactors
 * should be made to avoid a pretty gross template signature.
 */

static motor_class::Motor pipette_motor{
    configs::linear_motion_sys_config_by_axis(PIPETTE_TYPE), plunger_hw,
    motor_messages::MotionConstraints{.min_velocity = 1,
                                      .max_velocity = 2,
                                      .min_acceleration = 1,
                                      .max_acceleration = 2},
    motor_queue};

static auto driver_configs = interfaces::driver_config_by_axis(PIPETTE_TYPE);

extern "C" void plunger_callback() { plunger_interrupt.run_interrupt(); }

extern "C" void gear_callback() {
    // TODO implement the motor handler for the 96 channel
}

static sensors::hardware::SensorHardware pins_for_sensor_lt(gpio::PinConfig{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
    .port = GPIOB,
    .pin = GPIO_PIN_4,
    .active_setting = GPIO_PIN_RESET});
static sensors::hardware::SensorHardware pins_for_sensor_96(gpio::PinConfig{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
    .port = GPIOB,
    .pin = GPIO_PIN_5,
    .active_setting = GPIO_PIN_RESET});

// Unfortunately, these numbers need to be literals or defines
// to get the compile-time checks to work so we can't actually
// correctly rely on the hal to get these numbers - they need
// to be checked against current configuration. However, they are
// - clock input is 110MHz assuming the CAN is clocked from sysclk
// - 455ns requested time quantum yields a 454ns actual
// - 275.33KHz bitrate
// - 87.5% sample point
// Should drive
// segment 1 = 8 quanta
// segment 2 = 1 quantum

// For the exact timing values these generate see
// can/tests/test_bit_timings.cpp
static constexpr auto can_bit_timings =
    can::bit_timings::BitTimings<110 * can::bit_timings::MHZ, 455, 275330,
                                 875>{};

auto main() -> int {
    HardwareInit();
    RCC_Peripheral_Clock_Select();
    MX_ICACHE_Init();
    utility_gpio_init();
    adc_init();
    initialize_enc(PIPETTE_TYPE);
    auto id = pipette_mounts::detect_id();

    i2c_setup(&i2chandler_struct);
    i2c_comms3.set_handle(i2chandler_struct.i2c3);
    i2c_comms1.set_handle(i2chandler_struct.i2c1);

    if (initialize_spi() != HAL_OK) {
        Error_Handler();
    }

    app_update_clear_flags();

    can_start(can_bit_timings.clock_divider, can_bit_timings.segment_1_quanta,
              can_bit_timings.segment_2_quanta,
              can_bit_timings.max_sync_jump_width);

    central_tasks::start_tasks(can_bus_1, id);
    peripheral_tasks::start_tasks(i2c_comms3, i2c_comms1, spi_comms);

    if (PIPETTE_TYPE == NINETY_SIX_CHANNEL) {
        sensor_tasks::start_tasks(*central_tasks::get_tasks().can_writer,
                                  peripheral_tasks::get_i2c3_writer(),
                                  peripheral_tasks::get_i2c1_writer(),
                                  peripheral_tasks::get_i2c1_poller_client(),
                                  pins_for_sensor_96, id);

        initialize_linear_timer(plunger_callback);
        initialize_gear_timer(gear_callback);
        linear_motor_tasks::start_tasks(*central_tasks::get_tasks().can_writer,
                                        pipette_motor.motion_controller,
                                        peripheral_tasks::get_spi_writer(),
                                        driver_configs, id);
        // todo update with correct motion controller.
        gear_motor_tasks::start_tasks(*central_tasks::get_tasks().can_writer,
                                      pipette_motor.motion_controller,
                                      peripheral_tasks::get_spi_writer(),
                                      driver_configs, id);
    } else {
        sensor_tasks::start_tasks(*central_tasks::get_tasks().can_writer,
                                  peripheral_tasks::get_i2c3_writer(),
                                  peripheral_tasks::get_i2c1_writer(),
                                  peripheral_tasks::get_i2c1_poller_client(),
                                  pins_for_sensor_lt, id);

        initialize_linear_timer(plunger_callback);
        linear_motor_tasks::start_tasks(*central_tasks::get_tasks().can_writer,
                                        pipette_motor.motion_controller,
                                        peripheral_tasks::get_spi_writer(),
                                        driver_configs, id);
    }

    iWatchdog.start(6);

    vTaskStartScheduler();
}
