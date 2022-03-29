#include "gripper/core/interfaces.hpp"

#include "can/firmware/hal_can.h"
#include "can/firmware/hal_can_bus.hpp"
#include "common/core/freertos_message_queue.hpp"
#include "common/firmware/iwdg.hpp"
#include "common/firmware/spi_comms.hpp"
#include "gripper/core/interfaces.hpp"
#include "gripper/core/tasks.hpp"
#include "motor-control/core/motion_controller.hpp"
#include "motor-control/core/motor_interrupt_handler.hpp"
#include "motor-control/firmware/brushed_motor/driver_hardware.hpp"
#include "motor-control/firmware/brushed_motor_hardware.hpp"
#include "motor-control/firmware/motor_hardware.hpp"
#pragma GCC diagnostic push
// NOLINTNEXTLINE(clang-diagnostic-unknown-warning-option)
#pragma GCC diagnostic ignored "-Wvolatile"
#include "motor_hardware.h"
#pragma GCC diagnostic pop

static auto iWatchdog = iwdg::IndependentWatchDog{};

/**
 * The SPI configuration.
 */
static spi::SPI_interface SPI_intf = {
    .SPI_handle = &hspi2,
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
    .GPIO_handle = GPIOB,
    .pin = GPIO_PIN_12,
};

/**
 * The SPI interface.
 */
static spi::Spi spi_comms(SPI_intf);

/**
 * Motor pin configuration.
 */
struct motion_controller::HardwareConfig motor_pins {
    .direction =
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
            .port = GPIOB,
            .pin = GPIO_PIN_10,
            .active_setting = GPIO_PIN_RESET},
    .step =
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
            .port = GPIOB,
            .pin = GPIO_PIN_1,
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
            .pin = GPIO_PIN_7,
            .active_setting = GPIO_PIN_SET},
    .led = {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
        .port = GPIOC,
        .pin = GPIO_PIN_6,
        .active_setting = GPIO_PIN_RESET},
};

/**
 * The motor hardware interface.
 */
static motor_hardware::MotorHardware motor_hardware_iface(motor_pins, &htim7);

/**
 * Motor driver configuration.
 */
static tmc2130::TMC2130DriverConfig MotorDriverConfigurations{
    .registers =
        {
            .gconfig = {.en_pwm_mode = 1},
            .ihold_irun = {.hold_current = 0x2,
                           .run_current = 0x19,
                           .hold_current_delay = 0x7},
            .tpowerdown = {},
            .tcoolthrs = {.threshold = 0},
            .thigh = {.threshold = 0xFFFFF},
            .chopconf = {.toff = 0x5,
                         .hstrt = 0x5,
                         .hend = 0x3,
                         .tbl = 0x2,
                         .mres = 0x4},
            .coolconf = {.sgt = 0x6},
        },
    .current_config = {
        .r_sense = 0.1,
        .v_sf = 0.325,
    }};

/**
 * The can bus.
 */
static auto canbus = hal_can_bus::HalCanBus(can_get_device_handle());

/**
 * The pending move queue
 */
static freertos_message_queue::FreeRTOSMessageQueue<motor_messages::Move>
    motor_queue("Motor Queue");

/**
 * The motor struct.
 */
static motor_class::Motor z_motor{
    spi_comms,
    lms::LinearMotionSystemConfig<lms::LeadScrewConfig>{
        .mech_config = lms::LeadScrewConfig{.lead_screw_pitch = 4},
        .steps_per_rev = 200,
        .microstep = 16},
    motor_hardware_iface,
    motor_messages::MotionConstraints{.min_velocity = 1,
                                      .max_velocity = 2,
                                      .min_acceleration = 1,
                                      .max_acceleration = 2},
    MotorDriverConfigurations,
    motor_queue};

/**
 * Handler of motor interrupts.
 */
static motor_handler::MotorInterruptHandler motor_interrupt(
    motor_queue, gripper_tasks::get_queues(), motor_hardware_iface);

/**
 * Timer callback.
 */
extern "C" void call_motor_handler(void) { motor_interrupt.run_interrupt(); }

/**
 * Brushed motor pin configuration.
 */
struct motor_hardware::BrushedHardwareConfig brushed_motor_pins {
    .pwm_1 =
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
            .tim = &htim1,
            .channel = TIM_CHANNEL_1},
    .pwm_2 =
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
            .tim = &htim3,
            .channel = TIM_CHANNEL_1},
    .enable =
        {  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
            .port = GPIOC,
            .pin = GPIO_PIN_11,
            .active_setting = GPIO_PIN_SET},
    .limit_switch = {  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
        .port = GPIOC,
        .pin = GPIO_PIN_2,
        .active_setting = GPIO_PIN_SET},
};

/**
 * Brushed motor dac configuration.
 */
struct brushed_motor_driver::DacConfig dac_config {
    .dac_handle = &hdac1, .channel = DAC_CHANNEL_1,
    .data_algn = DAC_ALIGN_12B_R,
};
/**
 * The brushed motor hardware interface.
 */
static motor_hardware::BrushedMotorHardware brushed_motor_hardware_iface(
    brushed_motor_pins);

/**
 * The brushed motor driver hardware interface.
 */
static brushed_motor_driver::BrushedMotorDriver brushed_motor_driver_iface(
    dac_config, brushed_motor_driver::DriverConfig{.vref = 1.25}, update_pwm);

void interfaces::initialize() {
    // Initialize SPI
    if (initialize_spi() != HAL_OK) {
        Error_Handler();
    }

    initialize_timer(call_motor_handler);

    // Initialize DAC
    initialize_dac();

    // Start the can bus
    can_start();

    iWatchdog.start(6);
}

auto interfaces::get_can_bus() -> can_bus::CanBus& { return canbus; }

auto interfaces::get_spi() -> spi::SpiDeviceBase& { return spi_comms; }

auto interfaces::get_motor_hardware_iface()
    -> motor_hardware::MotorHardwareIface& {
    return motor_hardware_iface;
}

auto interfaces::get_z_motor() -> motor_class::Motor<lms::LeadScrewConfig>& {
    return z_motor;
}

auto interfaces::get_brushed_motor_hardware_iface()
    -> motor_hardware::BrushedMotorHardwareIface& {
    return brushed_motor_hardware_iface;
}

auto interfaces::get_brushed_motor_driver_hardware_iface()
    -> brushed_motor_driver::BrushedMotorDriverIface& {
    return brushed_motor_driver_iface;
}
