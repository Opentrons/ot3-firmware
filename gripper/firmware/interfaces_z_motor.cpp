
#include "gripper/core/can_task.hpp"
#include "gripper/core/interfaces.hpp"
#include "motor-control/core/stepper_motor/motion_controller.hpp"
#include "motor-control/core/stepper_motor/motor_interrupt_handler.hpp"
#include "motor-control/core/stepper_motor/tmc2130.hpp"
#include "motor-control/core/tasks/motor_hardware_task.hpp"
#include "motor-control/firmware/stepper_motor/motor_hardware.hpp"
#include "spi/firmware/spi_comms.hpp"

#pragma GCC diagnostic push
// NOLINTNEXTLINE(clang-diagnostic-unknown-warning-option)
#pragma GCC diagnostic ignored "-Wvolatile"
#include "motor_encoder_hardware.h"
#include "motor_hardware.h"
#pragma GCC diagnostic pop

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
struct motion_controller::HardwareConfig motor_pins {
    .direction =
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
            .port = GPIOB,
            .pin = GPIO_PIN_10,
            .active_setting = GPIO_PIN_SET},
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

/**
 * The motor hardware interface.
 */
static motor_hardware::MotorHardware motor_hardware_iface(motor_pins, &htim7,
                                                          nullptr);

/**
 * Motor driver configuration.
 */
static tmc2130::configs::TMC2130DriverConfig MotorDriverConfigurations{
    .registers = {.gconfig = {.en_pwm_mode = 0x0, .stop_enable = 0x1},
                  .ihold_irun = {.hold_current = 0x2,  // 0.177A
                                 .run_current = 0xA,   // 0.648A
                                 .hold_current_delay = 0x7},
                  .tpowerdown = {},
                  .tcoolthrs = {.threshold = 0},
                  .thigh = {.threshold = 0xFFFFF},
                  .chopconf = {.toff = 0x5,
                               .hstrt = 0x5,
                               .hend = 0x3,
                               .tbl = 0x2,
                               .mres = 0x3},
                  .coolconf = {.sgt = 0x6},
                  .pwmconf = {.pwm_ampl = 0x80,
                              .pwm_grad = 0x04,
                              .pwm_freq = 0x1,
                              .pwm_autoscale = 0x01}},
    .current_config =
        {
            .r_sense = 0.1,
            .v_sf = 0.32,
        },
    .chip_select = {
        .cs_pin = GPIO_PIN_12,
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
        .GPIO_handle = GPIOB,
    }};

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
static motor_class::Motor z_motor{
    lms::LinearMotionSystemConfig<lms::LeadScrewConfig>{
        .mech_config = lms::LeadScrewConfig{.lead_screw_pitch = 12,
                                            .gear_reduction_ratio = 1.8},
        .steps_per_rev = 200,
        .microstep = 32,
        .encoder_pulses_per_rev = 0},
    motor_hardware_iface,
    motor_messages::MotionConstraints{.min_velocity = 1,
                                      .max_velocity = 2,
                                      .min_acceleration = 1,
                                      .max_acceleration = 2},
    motor_queue,
    update_position_queue,
    true};

// There is no encoder so the ratio doesn't matter
static stall_check::StallCheck stallcheck(0, 0, 0);

/**
 * Handler of motor interrupts.
 */
static motor_handler::MotorInterruptHandler motor_interrupt(
    motor_queue, gripper_tasks::z_tasks::get_queues(), motor_hardware_iface,
    stallcheck, update_position_queue);

/**
 * Timer callback.
 */
extern "C" void call_motor_handler(void) { motor_interrupt.run_interrupt(); }

void z_motor_iface::initialize() {
    if (initialize_spi() != HAL_OK) {
        Error_Handler();
    }

    initialize_timer(call_motor_handler);
}

auto z_motor_iface::get_spi() -> spi::hardware::SpiDeviceBase& {
    return spi_comms;
}

auto z_motor_iface::get_z_motor() -> motor_class::Motor<lms::LeadScrewConfig, z_motor_iface::DefinedZMotorHardware>& {
    return z_motor;
}

auto z_motor_iface::get_tmc2130_driver_configs()
    -> tmc2130::configs::TMC2130DriverConfig& {
    return MotorDriverConfigurations;
}

static auto zmh_tsk = motor_hardware_task::MotorHardwareTask{
    &motor_hardware_iface, "z motor hardware task"};
auto z_motor_iface::get_z_motor_hardware_task()
    -> motor_hardware_task::MotorHardwareTask<z_motor_iface::DefinedZMotorHardware>& {
    return zmh_tsk;
}
