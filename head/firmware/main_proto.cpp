#include <array>
#include <cstdio>
#include <cstring>

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
#include "head/firmware/adc.h"
#include "motor_hardware.h"
#include "stm32g4xx_hal.h"
#include "stm32g4xx_hal_conf.h"
#include "utility_hardware.h"
#pragma GCC diagnostic pop
#include "can/core/bit_timings.hpp"
#include "can/firmware/hal_can_bus.hpp"
#include "common/core/freertos_timer.hpp"
#include "common/firmware/clocking.h"
#include "common/firmware/gpio.hpp"
#include "head/core/presence_sensing_driver.hpp"
#include "head/core/queues.hpp"
#include "head/core/tasks_proto.hpp"
#include "head/firmware/adc_comms.hpp"
#include "motor-control/core/linear_motion_system.hpp"
#include "motor-control/core/stepper_motor/motor.hpp"
#include "motor-control/core/stepper_motor/motor_interrupt_handler.hpp"
#include "motor-control/core/stepper_motor/tmc2130.hpp"
#include "motor-control/firmware/stepper_motor/motor_hardware.hpp"
#include "spi/firmware/spi_comms.hpp"

static auto iWatchdog = iwdg::IndependentWatchDog{};

static auto can_bus_1 = can::hal::bus::HalCanBus(
    can_get_device_handle(),
    gpio::PinConfig{// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                    .port = GPIOB,
                    .pin = GPIO_PIN_6,
                    .active_setting = GPIO_PIN_RESET});

static freertos_message_queue::FreeRTOSMessageQueue<motor_messages::Move>
    motor_queue_left("Motor Queue Left");

static freertos_message_queue::FreeRTOSMessageQueue<motor_messages::Move>
    motor_queue_right("Motor Queue Right");

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

struct motor_hardware::HardwareConfig pin_configurations_left {
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

struct motor_hardware::HardwareConfig pin_configurations_right {
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
            .active_setting = GPIO_PIN_RESET},
    .limit_switch =
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
            .port = GPIOB,
            .pin = GPIO_PIN_9,
            .active_setting = GPIO_PIN_SET},
    .led =
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
            .port = GPIOB,
            .pin = GPIO_PIN_6,
            .active_setting = GPIO_PIN_RESET},
    .sync_in = {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
        .port = GPIOA,
        .pin = GPIO_PIN_8,
        .active_setting = GPIO_PIN_RESET}
};

// TODO clean up the head main file by using interfaces.
static tmc2130::configs::TMC2130DriverConfig motor_driver_configs_right{
    .registers =
        {
            .gconfig = {.en_pwm_mode = 1},
            .ihold_irun = {.hold_current = 0xB,
                           .run_current = 0x19,
                           .hold_current_delay = 0x7},
            .tcoolthrs = {.threshold = 0},
            .thigh = {.threshold = 0xFFFFF},
            .chopconf = {.toff = 0x5,
                         .hstrt = 0x5,
                         .hend = 0x3,
                         .tbl = 0x2,
                         .mres = 0x4},
            .coolconf = {.sgt = 0x6},
        },
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

static tmc2130::configs::TMC2130DriverConfig motor_driver_configs_left{
    .registers =
        {
            .gconfig = {.en_pwm_mode = 1},
            .ihold_irun = {.hold_current = 0xB,
                           .run_current = 0x19,
                           .hold_current_delay = 0x7},
            .tcoolthrs = {.threshold = 0},
            .thigh = {.threshold = 0xFFFFF},
            .chopconf = {.toff = 0x5,
                         .hstrt = 0x5,
                         .hend = 0x3,
                         .tbl = 0x2,
                         .mres = 0x4},
            .coolconf = {.sgt = 0x6},
        },
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

/**
 * TODO: This motor class is only used in motor handler and should be
 * instantiated inside of the MotorHandler class. However, some refactors
 * should be made to avoid a pretty gross template signature.
 */

static motor_hardware::MotorHardware motor_hardware_right(
    pin_configurations_right, &htim7, &htim2);
static motor_handler::MotorInterruptHandler motor_interrupt_right(
    motor_queue_right, head_tasks::get_right_queues(), motor_hardware_right);

static motor_class::Motor motor_right{
    lms::LinearMotionSystemConfig<lms::LeadScrewConfig>{
        .mech_config = lms::LeadScrewConfig{.lead_screw_pitch = 12.0},
        .steps_per_rev = 200.0,
        .microstep = 16.0,
        .encoder_pulses_per_rev = 1000.0},
    motor_hardware_right,
    motor_messages::MotionConstraints{.min_velocity = 1,
                                      .max_velocity = 2,
                                      .min_acceleration = 1,
                                      .max_acceleration = 2},
    motor_queue_right};

static motor_hardware::MotorHardware motor_hardware_left(
    pin_configurations_left, &htim7, &htim3);
static motor_handler::MotorInterruptHandler motor_interrupt_left(
    motor_queue_left, head_tasks::get_left_queues(), motor_hardware_left);

static motor_class::Motor motor_left{
    lms::LinearMotionSystemConfig<lms::LeadScrewConfig>{
        .mech_config = lms::LeadScrewConfig{.lead_screw_pitch = 12.0},
        .steps_per_rev = 200.0,
        .microstep = 16.0,
        .encoder_pulses_per_rev = 1000.0},
    motor_hardware_left,
    motor_messages::MotionConstraints{.min_velocity = 1,
                                      .max_velocity = 2,
                                      .min_acceleration = 1,
                                      .max_acceleration = 2},
    motor_queue_left};

extern "C" void motor_callback_glue() {
    motor_interrupt_left.run_interrupt();
    motor_interrupt_right.run_interrupt();
}

extern "C" void left_enc_overflow_callback_glue(int32_t direction) {
    motor_hardware_left.encoder_overflow(direction);
}

extern "C" void right_enc_overflow_callback_glue(int32_t direction) {
    motor_hardware_right.encoder_overflow(direction);
}

static auto ADC_comms = adc::ADC(get_adc1_handle(), get_adc2_handle());

static auto psd = presence_sensing_driver::PresenceSensingDriver{ADC_comms};

auto timer_for_notifier = freertos_timer::FreeRTOSTimer(
    "timer for notifier", ([] {
        auto* presence_sensing_task =
            head_tasks::get_tasks().presence_sensing_driver_task;
        if (presence_sensing_task != nullptr) {
            presence_sensing_task->notifier_callback();
        }
    }),
    100);

// Unfortunately, these numbers need to be literals or defines
// to get the compile-time checks to work so we can't actually
// correctly rely on the hal to get these numbers - they need
// to be checked against current configuration. However, they are
// - clock input is 85MHz assuming the CAN is clocked from PCLK1
// which has a clock divider of 2, and the system clock is 170MHZ
// - 50ns time quantum
// - 250KHz bitrate requested yields 250312KHz actual
// - 88.2% sample point
// Should drive
// segment 1 = 73 quanta
// segment 2 = 11 quanta

// For the exact timing values these generate see
// can/tests/test_bit_timings.cpp
static constexpr auto can_bit_timings =
    can::bit_timings::BitTimings<170 * can::bit_timings::MHZ, 100,
                                 500 * can::bit_timings::KHZ, 800>{};

auto main() -> int {
    HardwareInit();
    RCC_Peripheral_Clock_Select();

    app_update_clear_flags();
    initialize_timer(motor_callback_glue, left_enc_overflow_callback_glue,
                     right_enc_overflow_callback_glue);

    if (initialize_spi(&hspi2) != HAL_OK) {
        Error_Handler();
    }
    if (initialize_spi(&hspi3) != HAL_OK) {
        Error_Handler();
    }

    utility_gpio_init();
    can_bus_1.start(can_bit_timings);
    head_tasks::start_tasks(can_bus_1, motor_left.motion_controller,
                            motor_right.motion_controller, psd, spi_comms2,
                            spi_comms3, motor_driver_configs_left,
                            motor_driver_configs_right);

    timer_for_notifier.start();

    iWatchdog.start(6);

    vTaskStartScheduler();
}
