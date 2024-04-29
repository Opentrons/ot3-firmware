
#include "gripper/core/can_task.hpp"
#include "gripper/core/interfaces.hpp"
#include "gripper/core/utils.hpp"
#include "gripper/firmware/eeprom_keys.hpp"
#include "gripper/firmware/utility_gpio.h"
#include "motor-control/core/stepper_motor/motion_controller.hpp"
#include "motor-control/core/stepper_motor/motor_encoder_background_timer.hpp"
#include "motor-control/core/stepper_motor/motor_interrupt_handler.hpp"
#include "motor-control/core/stepper_motor/tmc2130.hpp"
#include "motor-control/core/tasks/motor_hardware_task.hpp"
#include "motor-control/firmware/stepper_motor/motor_hardware.hpp"
#include "spi/firmware/spi_comms.hpp"

#pragma GCC diagnostic push
// NOLINTNEXTLINE(clang-diagnostic-unknown-warning-option)
#pragma GCC diagnostic ignored "-Wvolatile"
#include "motor_hardware.h"
#pragma GCC diagnostic pop

/**
 * The SPI configuration.
 */
static spi::hardware::SPI_interface SPI_intf = {
    .SPI_handle = &hspi2,
};

struct motor_hardware::UsageEEpromConfig z_usage_config {
    std::array<UsageRequestSet, 2> {
        UsageRequestSet{
            .eeprom_key = Z_MOTOR_DIST_KEY,
            .type_key =
                uint16_t(can::ids::MotorUsageValueType::linear_motor_distance),
            .length = usage_storage_task::distance_data_usage_len},
            UsageRequestSet {
            .eeprom_key = Z_ERROR_COUNT_KEY,
            .type_key =
                uint16_t(can::ids::MotorUsageValueType::total_error_count),
            .length = usage_storage_task::error_count_usage_len
        }
    }
};

/**
 * The SPI interface.
 */
static spi::hardware::Spi spi_comms(SPI_intf);

#if PCBA_PRIMARY_REVISION == 'b' || PCBA_PRIMARY_REVISION == 'a'
static void* enc_handle = nullptr;
static constexpr float encoder_pulses = 0.0;
static constexpr std::optional<gpio::PinConfig> ebrake = std::nullopt;
static constexpr uint8_t use_stop_enable = 0x1;
#else
static constexpr uint8_t use_stop_enable = 0x0;
static constexpr void* enc_handle = &htim8;
static constexpr float encoder_pulses = 1024.0;
static gpio::PinConfig ebrake = {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
    .port = EBRAKE_PORT,
    .pin = EBRAKE_PIN,
    .active_setting = GPIO_PIN_RESET};
#endif

/**
 * Motor pin configuration.
 */
struct motion_controller::HardwareConfig motor_pins {
    .direction =
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
            .port = Z_MOT_STEPDIR_PORT,
            .pin = Z_MOT_DIR_PIN,
            .active_setting = GPIO_PIN_SET},
    .step =
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
            .port = Z_MOT_STEPDIR_PORT,
            .pin = Z_MOT_STEP_PIN,
            .active_setting = GPIO_PIN_SET},
    .enable =
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
            .port = Z_MOT_ENABLE_PORT,
            .pin = Z_MOT_ENABLE_PIN,
            .active_setting = GPIO_PIN_SET},
    .limit_switch =
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
            .port = Z_LIM_SW_PORT,
            .pin = Z_LIM_SW_PIN,
            .active_setting = GPIO_PIN_SET},
    .led = {},
    .sync_in =
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
            .port = NSYNC_IN_PORT,
            .pin = NSYNC_IN_PIN,
            .active_setting = GPIO_PIN_RESET},
    .estop_in =
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
            .port = ESTOP_IN_PORT,
            .pin = ESTOP_IN_PIN,
            .active_setting = GPIO_PIN_RESET},
    .diag0 =
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
            .port = GPIOB,
            .pin = GPIO_PIN_2,
            .active_setting = GPIO_PIN_RESET},
    .ebrake = ebrake
};

/**
 * The motor hardware interface.
 */

static motor_hardware::MotorHardware motor_hardware_iface(motor_pins, &htim7,
                                                          enc_handle,
                                                          z_usage_config);

/**
 * Motor driver configuration.
 */
static tmc2130::configs::TMC2130DriverConfig MotorDriverConfigurations{
    .registers = {.gconfig = {.en_pwm_mode = 0x0,
                              .diag0_error = 1,
                              .stop_enable = use_stop_enable},
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
        .cs_pin = Z_MOT_DRIVE_CS,
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
        .GPIO_handle = Z_MOT_DRIVE_PORT,
    }};

/**
 * The pending move queue
 */
#ifdef USE_SENSOR_MOVE
static freertos_message_queue::FreeRTOSMessageQueue<motor_messages::SensorSyncMove>
    motor_queue("Motor Queue");
#else
static freertos_message_queue::FreeRTOSMessageQueue<motor_messages::Move>
    motor_queue("Motor Queue");
#endif

static freertos_message_queue::FreeRTOSMessageQueue<
    can::messages::UpdateMotorPositionEstimationRequest>
    update_position_queue("Position Queue");

static lms::LinearMotionSystemConfig<lms::LeadScrewConfig> linear_config{
    .mech_config = lms::LeadScrewConfig{.lead_screw_pitch = 12,
                                        .gear_reduction_ratio = 1.8},
    .steps_per_rev = 200,
    .microstep = 32,
    .encoder_pulses_per_rev = encoder_pulses};

#if PCBA_PRIMARY_REVISION == 'b' || PCBA_PRIMARY_REVISION == 'a'
static auto stallcheck = stall_check::StallCheck(0, 0, 0);
#else
static auto stallcheck = stall_check::StallCheck(
    linear_config.get_encoder_pulses_per_mm() / 1000.0F,
    linear_config.get_usteps_per_mm() / 1000.0F, utils::STALL_THRESHOLD_UM);
#endif

/**
 * The motor struct.
 */
static motor_class::Motor z_motor{
    linear_config,
    motor_hardware_iface,
    motor_messages::MotionConstraints{.min_velocity = 1,
                                      .max_velocity = 2,
                                      .min_acceleration = 1,
                                      .max_acceleration = 2},
    motor_queue,
    update_position_queue,
    true};

/**
 * Handler of motor interrupts.
 */
static motor_handler::MotorInterruptHandler motor_interrupt(
    motor_queue, gripper_tasks::z_tasks::get_queues(),
    gripper_tasks::z_tasks::get_queues(), motor_hardware_iface, stallcheck,
    update_position_queue, gripper_tasks::get_main_queues());

static auto encoder_background_timer =
    motor_encoder::BackgroundTimer(motor_interrupt, motor_hardware_iface);

/**
 * Timer callback.
 */
extern "C" void call_motor_handler(void) { motor_interrupt.run_interrupt(); }
extern "C" void call_enc_handler(int32_t direction) {
    motor_hardware_iface.encoder_overflow(direction);
}

void z_motor_iface::initialize(diag0_handler* call_diag0_handler) {
    if (initialize_spi() != HAL_OK) {
        Error_Handler();
    }
    initialize_hardware_z();
    set_z_motor_timer_callback(call_motor_handler, call_diag0_handler,
                               call_enc_handler);
    encoder_background_timer.start();
}

auto z_motor_iface::get_spi() -> spi::hardware::SpiDeviceBase& {
    return spi_comms;
}

auto z_motor_iface::get_z_motor() -> motor_class::Motor<lms::LeadScrewConfig>& {
    return z_motor;
}

auto z_motor_iface::get_tmc2130_driver_configs()
    -> tmc2130::configs::TMC2130DriverConfig& {
    return MotorDriverConfigurations;
}

static auto zmh_tsk = motor_hardware_task::MotorHardwareTask{
    &motor_hardware_iface, "z motor hardware task"};
auto z_motor_iface::get_z_motor_hardware_task()
    -> motor_hardware_task::MotorHardwareTask& {
    return zmh_tsk;
}
