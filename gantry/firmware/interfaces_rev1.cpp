#include "gantry/core/interfaces_rev1.hpp"

#include "can/core/bit_timings.hpp"
#include "can/firmware/hal_can.h"
#include "can/firmware/hal_can_bus.hpp"
#include "common/core/freertos_message_queue.hpp"
#include "common/firmware/gpio.hpp"
#include "common/firmware/iwdg.hpp"
#include "gantry/core/axis_type.h"
#include "gantry/core/queues.hpp"
#include "gantry/core/utils.hpp"
#include "motor-control/core/stepper_motor/motion_controller.hpp"
#include "motor-control/core/stepper_motor/motor_interrupt_handler.hpp"
#include "motor-control/firmware/stepper_motor/motor_hardware.hpp"
#include "spi/firmware/spi_comms.hpp"
#pragma GCC diagnostic push
// NOLINTNEXTLINE(clang-diagnostic-unknown-warning-option)
#pragma GCC diagnostic ignored "-Wvolatile"
#include "motor_hardware.h"
#pragma GCC diagnostic pop

static auto iWatchdog = iwdg::IndependentWatchDog{};

/**
 * The SPI configuration.
 */
static spi::hardware::SPI_interface SPI_intf = {
    .SPI_handle = &hspi2,
};

/**
 * The SPI interface.
 */
static spi::hardware::Spi spi_comms(SPI_intf);

/**
 * Motor pin configuration.
 */
struct motion_controller::HardwareConfig motor_pins_x {
    .direction =
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
            .port = GPIOB,
            .pin = GPIO_PIN_1,
            .active_setting = GPIO_PIN_RESET},
    .step =
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
            .port = GPIOC,
            .pin = GPIO_PIN_8,
            .active_setting = GPIO_PIN_SET},
    .enable =
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
            .port = GPIOA,
            .pin = GPIO_PIN_9,
            .active_setting = GPIO_PIN_SET},
    .limit_switch =
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
            .port = GPIOC,
            .pin = GPIO_PIN_2,
            .active_setting = GPIO_PIN_SET},
    .led = {},
    .sync_in = {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
        .port = GPIOB,
        .pin = GPIO_PIN_7,
        .active_setting = GPIO_PIN_RESET}
};

struct motion_controller::HardwareConfig motor_pins_y {
    .direction =
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
            .port = GPIOB,
            .pin = GPIO_PIN_1,
            .active_setting = GPIO_PIN_SET},
    .step =
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
            .port = GPIOC,
            .pin = GPIO_PIN_8,
            .active_setting = GPIO_PIN_SET},
    .enable =
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
            .port = GPIOA,
            .pin = GPIO_PIN_9,
            .active_setting = GPIO_PIN_SET},
    .limit_switch =
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
            .port = GPIOC,
            .pin = GPIO_PIN_2,
            .active_setting = GPIO_PIN_SET},
    .led = {},
    .sync_in = {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
        .port = GPIOB,
        .pin = GPIO_PIN_7,
        .active_setting = GPIO_PIN_RESET}
};

static tmc2160::configs::TMC2160DriverConfig motor_driver_config{
    .registers =
        {
            .gconfig = {.en_pwm_mode = 1},
            .ihold_irun = {.hold_current = 16,
                           .run_current = 31,
                           .hold_current_delay = 0x7},
            .tcoolthrs = {.threshold = 0},
            .thigh = {.threshold = 0xFFFFF},
            .chopconf = {.toff = 0x5,
                         .hstrt = 0x4,
                         .hend = 0x0,
                         .tbl = 0x2,
                         .tpfd = 0x4,
                         .mres = 0x4},
            .coolconf = {.sgt = 0x6},
            .pwmconf = {.pwm_autoscale = 0x1,
                        .pwm_autograd = 0x1,
                        .}
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

/**
 * The motor hardware interface.
 */
static motor_hardware::MotorHardware motor_hardware_iface(
    (get_axis_type() == gantry_x) ? motor_pins_x : motor_pins_y, &htim7,
    nullptr);

/**
 * The can bus.
 */
static auto canbus = can::hal::bus::HalCanBus(
    can_get_device_handle(),
    gpio::PinConfig{// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                    .port = GPIOB,
                    .pin = GPIO_PIN_11,
                    .active_setting = GPIO_PIN_RESET});

/**
 * The pending move queue
 */
static freertos_message_queue::FreeRTOSMessageQueue<motor_messages::Move>
    motor_queue("Motor Queue");

/**
 * The motor struct.
 */
static motor_class::Motor motor{
    utils::linear_motion_system_config(), motor_hardware_iface,
    motor_messages::MotionConstraints{.min_velocity = 1,
                                      .max_velocity = 2,
                                      .min_acceleration = 1,
                                      .max_acceleration = 2},
    motor_queue};

/**
 * Handler of motor interrupts.
 */
static motor_handler::MotorInterruptHandler motor_interrupt(
    motor_queue, gantry::queues::get_queues(), motor_hardware_iface);

/**
 * Timer callback.
 */
extern "C" void call_motor_handler(void) { motor_interrupt.run_interrupt(); }

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

void interfaces::initialize() {
    // Initialize SPI
    if (initialize_spi(get_axis_type()) != HAL_OK) {
        Error_Handler();
    }

    initialize_timer(call_motor_handler);

    // Start the can bus
    canbus.start(can_bit_timings);

    iWatchdog.start(6);
}

auto interfaces::get_can_bus() -> can::bus::CanBus& { return canbus; }

auto interfaces::get_spi() -> spi::hardware::SpiDeviceBase& {
    return spi_comms;
}

auto interfaces::get_motor_hardware_iface()
    -> motor_hardware::StepperMotorHardwareIface& {
    return motor_hardware_iface;
}

auto interfaces::get_motor() -> motor_class::Motor<lms::BeltConfig>& {
    return motor;
}

auto interfaces::get_driver_config() -> tmc2160::configs::TMC2160DriverConfig& {
    return motor_driver_config;
}
