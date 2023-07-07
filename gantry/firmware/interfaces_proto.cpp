#include "gantry/core/interfaces_proto.hpp"

#include "can/core/bit_timings.hpp"
#include "can/firmware/hal_can.h"
#include "can/firmware/hal_can_bus.hpp"
#include "common/core/freertos_message_queue.hpp"
#include "common/firmware/gpio.hpp"
#include "common/firmware/iwdg.hpp"
#include "gantry/core/axis_type.h"
#include "gantry/core/queues.hpp"
#include "gantry/core/utils.hpp"
#include "gantry/firmware/eeprom_keys.hpp"
#include "motor-control/core/stepper_motor/motion_controller.hpp"
#include "motor-control/core/stepper_motor/motor_encoder_background_timer.hpp"
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

struct motor_hardware::UsageEEpromConfig usage_config {
    std::array<UsageRequestSet, 2> {
        UsageRequestSet{
            .eeprom_key = AXIS_DISTANCE_KEY,
            .type_key =
                uint16_t(can::ids::MotorUsageValueType::linear_motor_distance),
            .length = usage_storage_task::distance_data_usage_len},
            UsageRequestSet {
            .eeprom_key = ERROR_COUNT_KEY,
            .type_key =
                uint16_t(can::ids::MotorUsageValueType::total_error_count),
            .length = usage_storage_task::error_count_usage_len
        }
    }
};

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
    .sync_in =
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
            .port = GPIOB,
            .pin = GPIO_PIN_7,
            .active_setting = GPIO_PIN_RESET},
    .estop_in = {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
        .port = GPIOA,
        .pin = GPIO_PIN_10,
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
    .sync_in =
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
            .port = GPIOB,
            .pin = GPIO_PIN_5,
            .active_setting = GPIO_PIN_RESET},
    .estop_in = {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
        .port = GPIOA,
        .pin = GPIO_PIN_10,
        .active_setting = GPIO_PIN_RESET}
};

static tmc2130::configs::TMC2130DriverConfig gantry_x_driver_configs{
    .registers = {.gconfig = {.en_pwm_mode = 1},
                  .ihold_irun = {.hold_current = 0x2,
                                 .run_current = 0x18,
                                 .hold_current_delay = 0x7},
                  .thigh = {.threshold = 0xFFFFF},
                  .chopconf = {.toff = 0x5,
                               .hstrt = 0x5,
                               .hend = 0x3,
                               .tbl = 0x2,
                               .mres = 0x2},
                  .coolconf = {.sgt = 0x6}},
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

static tmc2130::configs::TMC2130DriverConfig gantry_y_driver_configs{
    .registers = {.gconfig = {.en_pwm_mode = 1},
                  .ihold_irun = {.hold_current = 0x2,
                                 .run_current = 0x18,
                                 .hold_current_delay = 0x7},
                  .thigh = {.threshold = 0xFFFFF},
                  .chopconf = {.toff = 0x5,
                               .hstrt = 0x5,
                               .hend = 0x3,
                               .tbl = 0x2,
                               .mres = 0x2},
                  .coolconf = {.sgt = 0x6}},
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
    nullptr, usage_config);

/**
 * The motor driver config.
 */
static tmc2130::configs::TMC2130DriverConfig motor_driver_config =
    (get_axis_type() == gantry_x) ? gantry_x_driver_configs
                                  : gantry_y_driver_configs;

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

static freertos_message_queue::FreeRTOSMessageQueue<
    can::messages::UpdateMotorPositionEstimationRequest>
    update_position_queue("Position Queue");

/**
 * The motor struct.
 */
static motor_class::Motor motor{
    utils::linear_motion_system_config(), motor_hardware_iface,
    motor_messages::MotionConstraints{.min_velocity = 1,
                                      .max_velocity = 2,
                                      .min_acceleration = 1,
                                      .max_acceleration = 2},
    motor_queue, update_position_queue};

static stall_check::StallCheck stallcheck(
    utils::linear_motion_system_config().get_encoder_pulses_per_mm() / 1000.0F,
    utils::linear_motion_system_config().get_usteps_per_mm() / 1000.0F,
    utils::STALL_THRESHOLD_UM);

/**
 * Handler of motor interrupts.
 */
static motor_handler::MotorInterruptHandler motor_interrupt(
    motor_queue, gantry::queues::get_queues(), motor_hardware_iface, stallcheck,
    update_position_queue);

static auto encoder_background_timer =
    motor_encoder::BackgroundTimer(motor_interrupt, motor_hardware_iface);

/**
 * Timer callback.
 */
extern "C" void call_motor_handler(void) { motor_interrupt.run_interrupt(); }

/**
 * Encoder overflow callback.
 */
extern "C" void enc_overflow_callback(int32_t direction) {
    motor_hardware_iface.encoder_overflow(direction);
}
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

    initialize_timer(call_motor_handler, enc_overflow_callback);

    // Start the can bus
    canbus.start(can_bit_timings);

    encoder_background_timer.start();

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

auto interfaces::get_driver_config() -> tmc2130::configs::TMC2130DriverConfig& {
    return motor_driver_config;
}

static auto mh_tsk = motor_hardware_task::MotorHardwareTask{
    &motor_hardware_iface, "motor hardware task"};
auto interfaces::get_motor_hardware_task()
    -> motor_hardware_task::MotorHardwareTask& {
    return mh_tsk;
}
