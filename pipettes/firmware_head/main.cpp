// clang-format off
#include "FreeRTOS.h"
#include "task.h"
#include "system_stm32g4xx.h"
#include "common/firmware/errors.h"
#include "common/core/app_update.h"
#include "common/firmware/iwdg.hpp"

// clang-format on
#pragma GCC diagnostic push
// NOLINTNEXTLINE(clang-diagnostic-unknown-warning-option)
#pragma GCC diagnostic ignored "-Wvolatile"
#include "motor_hardware.h"
#include "pipettes/firmware/i2c_setup.h"
#include "stm32g4xx_hal.h"
#include "stm32g4xx_hal_conf.h"
#include "utility_hardware.h"
#pragma GCC diagnostic pop
#include "can/core/bit_timings.hpp"
#include "can/firmware/hal_can_bus.hpp"
#include "common/core/freertos_timer.hpp"
#include "common/firmware/clocking.h"
#include "common/firmware/gpio.hpp"
#include "motor-control/core/linear_motion_system.hpp"
#include "motor-control/core/stepper_motor/motor.hpp"
#include "motor-control/core/stepper_motor/motor_interrupt_handler.hpp"
#include "motor-control/core/stepper_motor/tmc2160.hpp"
#include "motor-control/firmware/stepper_motor/motor_hardware.hpp"
#include "mount_detection.hpp"
#include "pipettes/core/central_tasks.hpp"
#include "pipettes/core/configs.hpp"
#include "pipettes/core/head_gear_tasks.hpp"
#include "pipettes/core/head_peripheral_tasks.hpp"
#include "pipettes/core/head_sensor_tasks.hpp"
#include "pipettes/core/pipette_type.h"
#include "pipettes/firmware/pipette_motor_hardware.hpp"
#include "spi/firmware/spi_comms.hpp"

constexpr auto PIPETTE_TYPE = get_pipette_type();

static auto iWatchdog = iwdg::IndependentWatchDog{};

static auto can_bus_1 = can::hal::bus::HalCanBus(
    can_get_device_handle(),
    gpio::PinConfig{// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                    .port = GPIOB,
                    .pin = GPIO_PIN_6,
                    .active_setting = GPIO_PIN_RESET});

spi::hardware::SPI_interface SPI_intf = {.SPI_handle = &hspi2};

/**
 * @brief SPI MSP Initialization
 * This function configures SPI for the Z/A axis motors
 * @param hspi: SPI handle pointer
 * @retval None
 */

spi::hardware::SPI_interface SPI_intf2 = {
    .SPI_handle = &hspi2,
};
static spi::hardware::Spi spi_comms2(SPI_intf2);

spi::hardware::SPI_interface SPI_intf3 = {
    .SPI_handle = &hspi3,
};
static spi::hardware::Spi spi_comms3(SPI_intf3);

static freertos_message_queue::FreeRTOSMessageQueue<
    motor_messages::GearMotorMove>
    motor_queue_left("Motor Queue Left");
static freertos_message_queue::FreeRTOSMessageQueue<
    motor_messages::GearMotorMove>
    motor_queue_right("Motor Queue Right");

struct pipette_motor_hardware::HardwareConfig pin_configurations_left {
    .direction =
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
            .port = GPIOC,
            .pin = GPIO_PIN_1,
            .active_setting = GPIO_PIN_RESET},
    .step =
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
            .port = GPIOC,
            .pin = GPIO_PIN_0,
            .active_setting = GPIO_PIN_SET},
    .enable =
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
            .port = GPIOC,
            .pin = GPIO_PIN_4,
            .active_setting = GPIO_PIN_SET},
    .limit_switch =
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
            .port = GPIOB,
            .pin = GPIO_PIN_7,
            .active_setting = GPIO_PIN_SET},
    .led = {},
    .sync_in = {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
        .port = GPIOA,
        .pin = GPIO_PIN_8,
        .active_setting = GPIO_PIN_RESET}
};

struct pipette_motor_hardware::HardwareConfig pin_configurations_right {
    .direction =
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
            .port = GPIOC,
            .pin = GPIO_PIN_7,
            .active_setting = GPIO_PIN_RESET},
    .step =
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
            .port = GPIOC,
            .pin = GPIO_PIN_6,
            .active_setting = GPIO_PIN_SET},
    .enable =
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
            .port = GPIOB,
            .pin = GPIO_PIN_11,
            .active_setting = GPIO_PIN_SET},
    .limit_switch =
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
            .port = GPIOB,
            .pin = GPIO_PIN_9,
            .active_setting = GPIO_PIN_SET},
    .led = {},
    .sync_in = {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
        .port = GPIOA,
        .pin = GPIO_PIN_8,
        .active_setting = GPIO_PIN_RESET}
};

// TODO clean up the head main file by using interfaces.
static tmc2160::configs::TMC2160DriverConfig motor_driver_configs_right{
    .registers = {.gconfig = {.en_pwm_mode = 1},
                  .ihold_irun = {.hold_current = 16,
                                 .run_current = 31,
                                 .hold_current_delay = 0x7},
                  .tcoolthrs = {.threshold = 0},
                  .thigh = {.threshold = 0xFFFFF},
                  .chopconf = {.toff = 0x3,
                               .hstrt = 0x5,
                               .hend = 0x2,
                               .tbl = 0x2,
                               .tpfd = 0x4,
                               .mres = 0x3},
                  .coolconf = {.sgt = 0x6},
                  .glob_scale = {.global_scaler = 0xA7}},
    .current_config =
        {
            .r_sense = 0.1,
            .v_sf = 0.325,
        },
    .chip_select{
        .cs_pin = GPIO_PIN_12,
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
        .GPIO_handle = GPIOB,
    }};

static tmc2160::configs::TMC2160DriverConfig motor_driver_configs_left{
    .registers = {.gconfig = {.en_pwm_mode = 1},
                  .ihold_irun = {.hold_current = 16,
                                 .run_current = 31,
                                 .hold_current_delay = 0x7},
                  .tcoolthrs = {.threshold = 0},
                  .thigh = {.threshold = 0xFFFFF},
                  .chopconf = {.toff = 0x3,
                               .hstrt = 0x5,
                               .hend = 0x2,
                               .tbl = 0x2,
                               .tpfd = 0x4,
                               .mres = 0x3},
                  .coolconf = {.sgt = 0x6},
                  .glob_scale = {.global_scaler = 0xA7}},
    .current_config =
        {
            .r_sense = 0.1,
            .v_sf = 0.325,
        },
    .chip_select{
        .cs_pin = GPIO_PIN_4,
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
        .GPIO_handle = GPIOA,
    }};

static constexpr auto can_bit_timings =
    can::bit_timings::BitTimings<170 * can::bit_timings::MHZ, 100,
                                 500 * can::bit_timings::KHZ, 800>{};

static pipette_motor_hardware::MotorHardware motor_hardware_right(
    pin_configurations_right, &htim7, &htim2);
static motor_handler::MotorInterruptHandler motor_interrupt_right(
    motor_queue_right, head_gear_tasks::get_right_gear_queues(),
    motor_hardware_right);

pipette_motion_controller::PipetteMotionController motor_right{
    configs::gear_motion_sys_config(), motor_hardware_right,
    motor_messages::MotionConstraints{.min_velocity = 1,
                                      .max_velocity = 2,
                                      .min_acceleration = 1,
                                      .max_acceleration = 2},
    motor_queue_right};

class PipetteEEPromHardwareIface
    : public eeprom::hardware_iface::EEPromHardwareIface {
  public:
    PipetteEEPromHardwareIface()
        : eeprom::hardware_iface::EEPromHardwareIface(
              eeprom::hardware_iface::EEPromChipType::ST_M24128) {}
    void set_write_protect(bool enable) final {
        if (enable) {
            disable_eeprom_write();
        } else {
            enable_eeprom_write();
        }
    }
};
static auto eeprom_hardware_iface = PipetteEEPromHardwareIface{};
static pipette_motor_hardware::MotorHardware motor_hardware_left(
    pin_configurations_left, &htim7, &htim3);
static motor_handler::MotorInterruptHandler motor_interrupt_left(
    motor_queue_left, head_gear_tasks::get_left_gear_queues(),
    motor_hardware_left);

pipette_motion_controller::PipetteMotionController motor_left{
    configs::gear_motion_sys_config(), motor_hardware_left,
    motor_messages::MotionConstraints{.min_velocity = 1,
                                      .max_velocity = 2,
                                      .min_acceleration = 1,
                                      .max_acceleration = 2},
    motor_queue_left};

extern "C" void gear_callback_wrapper() {
    motor_interrupt_left.run_interrupt();
    motor_interrupt_right.run_interrupt();
}

extern "C" void left_enc_overflow_callback_glue(int32_t) {}

extern "C" void right_enc_overflow_callback_glue(int32_t) {}

static auto i2c_comms3 = i2c::hardware::I2C();
static I2CHandlerStruct i2chandler_struct{};

auto main() -> int {
    HardwareInit();
    RCC_Peripheral_Clock_Select();

    auto id = pipette_mounts::detect_id();

    i2c_setup(&i2chandler_struct);
    i2c_comms3.set_handle(i2chandler_struct.i2c3);

    if (initialize_spi(&hspi2) != HAL_OK) {
        Error_Handler();
    }
    if (initialize_spi(&hspi3) != HAL_OK) {
        Error_Handler();
    }

    utility_gpio_init();

    app_update_clear_flags();

    can_bus_1.start(can_bit_timings);

    central_tasks::start_tasks(can_bus_1, id);
    head_peripheral_tasks::start_tasks(i2c_comms3, spi_comms2, spi_comms3);
    head_sensor_tasks::start_tasks(
        *central_tasks::get_tasks().can_writer,
        head_peripheral_tasks::get_i2c3_client(),
        head_peripheral_tasks::get_i2c3_poller_client(), id,
        eeprom_hardware_iface);

    initialize_gear_timer(gear_callback_wrapper,
                          left_enc_overflow_callback_glue,
                          right_enc_overflow_callback_glue);
    head_gear_tasks::start_tasks(
        *central_tasks::get_tasks().can_writer, id, motor_left, motor_right,
        head_peripheral_tasks::get_spi2_client(),
        head_peripheral_tasks::get_spi3_client(), motor_driver_configs_left,
        motor_driver_configs_right);

    iWatchdog.start(6);

    vTaskStartScheduler();
}
