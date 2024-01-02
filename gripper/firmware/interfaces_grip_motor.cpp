#include "gripper/core/can_task.hpp"
#include "gripper/core/interfaces.hpp"
#include "gripper/firmware/eeprom_keys.hpp"
#include "motor-control/core/brushed_motor/brushed_motor.hpp"
#include "motor-control/core/brushed_motor/brushed_motor_interrupt_handler.hpp"
#include "motor-control/core/brushed_motor/error_tolerance_config.hpp"
#include "motor-control/core/tasks/motor_hardware_task.hpp"
#include "motor-control/firmware/brushed_motor/brushed_motor_hardware.hpp"
#include "motor-control/firmware/brushed_motor/driver_hardware.hpp"
#pragma GCC diagnostic push
// NOLINTNEXTLINE(clang-diagnostic-unknown-warning-option)
#pragma GCC diagnostic ignored "-Wvolatile"
#include "motor_hardware.h"

#pragma GCC diagnostic pop

constexpr uint32_t PWM_MAX = 60;
constexpr uint32_t PWM_MIN = 7;
auto constexpr pwm_freq = double(GRIPPER_JAW_PWM_FREQ_HZ);

struct motor_hardware::UsageEEpromConfig brushed_usage_config {
    std::array<UsageRequestSet, 3> {
        UsageRequestSet{
            .eeprom_key = G_MOTOR_DIST_KEY,
            .type_key =
                uint16_t(can::ids::MotorUsageValueType::linear_motor_distance),
            .length = usage_storage_task::distance_data_usage_len},
            UsageRequestSet{
                .eeprom_key = G_MOTOR_FORCE_TIME_KEY,
                .type_key = uint16_t(
                    can::ids::MotorUsageValueType::force_application_time),
                .length = usage_storage_task::force_time_data_usage_len},
            UsageRequestSet {
            .eeprom_key = G_ERROR_COUNT_KEY,
            .type_key =
                uint16_t(can::ids::MotorUsageValueType::total_error_count),
            .length = usage_storage_task::error_count_usage_len
        }
    }
};

/**
 * Brushed motor pin configuration.
 */
struct motor_hardware::BrushedHardwareConfig brushed_motor_conf {
    .pwm_1 =
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
            .tim = &htim3,
            .channel = TIM_CHANNEL_1},
    .pwm_2 =
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
            .tim = &htim1,
            .channel = TIM_CHANNEL_1},
    .enable =
        {  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
            .port = G_MOT_ENABLE_PORT,
            .pin = G_MOT_ENABLE_PIN,
            .active_setting = GPIO_PIN_SET},
    .limit_switch =
        {  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
            .port = G_LIM_SW_PORT,
            .pin = G_LIM_SW_PIN,
            .active_setting = GPIO_PIN_SET},
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
    .encoder_interrupt_freq = pwm_freq,

    /* the expected behavior with these pid values is that the motor runs at
     * full power until it's about 2mm away and then it slows down on that
     * approach. final values as the error delta approachs 0.01mm is ~7
     * which is the floor of pwm values that still move the gripper
     * this 7 is from the windup limits, but over very short moves that are less
     * than 5 mm this doesn't wind up all the way to 7 but the we use the
     * pwm_active_duty_clamp method to make sure that no matter the output that
     * we clamp the pwm to something within the acceptable values of the motor
     *
     * the most significant value here is the proportional gain
     * when the distance error is > 2mm (12210 encoder ticks)
     * the proportional term will be > 100 and reduce linearly
     * the derivative gain here was chosen so that it helps make the slowdown
     * when the distance error < 2mm but the term's significane drops off
     * pwm drops below 20 (20 pwm is about 2 encoder pulses/ interrupt)
     * (0.000015 * 2 / (1/32000)) ~= 1
     *
     * and finally the intergral gain helps ramp up to the -7 or +7 during moves
     * it needs moves of ~5mm to hit this but the clamping of the pwm control
     * will make sure that the total output is still floored to 7
     */
        .pid_kp = 0.008, .pid_ki = 0.0045, .pid_kd = 0.000015,
    .wl_high = PWM_MIN, .wl_low = (-1 * PWM_MIN)
};

/**
 * The pending move queue
 */
static freertos_message_queue::FreeRTOSMessageQueue<motor_messages::BrushedMove>
    motor_queue("Brushed Motor Queue");

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
    brushed_motor_conf, &htim2, brushed_usage_config, &htim15);

/**
 * The brushed motor driver hardware interface.
 */
static brushed_motor_driver::BrushedMotorDriver brushed_motor_driver_iface(
    dac_config,
    brushed_motor_driver::DriverConfig{
        .vref = 2.0, .pwm_min = PWM_MIN, .pwm_max = PWM_MAX},
    update_pwm);

static lms::LinearMotionSystemConfig<lms::GearBoxConfig> gear_config{
    .mech_config =
        lms::GearBoxConfig{.gear_diameter = 9, .gear_reduction_ratio = 103.81},
    .steps_per_rev = 0,
    .microstep = 0,
    .encoder_pulses_per_rev = 512};

static error_tolerance_config::BrushedMotorErrorTolerance error_conf(
    gear_config, pwm_freq);

static brushed_motor::BrushedMotor grip_motor(gear_config,
                                              brushed_motor_hardware_iface,
                                              brushed_motor_driver_iface,
                                              motor_queue, error_conf);

/**
 * Handler of brushed motor interrupts.
 */
static brushed_motor_handler::BrushedMotorInterruptHandler
    brushed_motor_interrupt(motor_queue, gripper_tasks::g_tasks::get_queues(),
                            brushed_motor_hardware_iface,
                            brushed_motor_driver_iface, error_conf);

extern "C" void call_brushed_motor_handler(void) {
    brushed_motor_interrupt.run_interrupt();
}

extern "C" void gripper_enc_overflow_callback_glue(int32_t direction) {
    brushed_motor_hardware_iface.encoder_overflow(direction);
}

extern "C" void gripper_enc_idle_state_callback_glue(bool val) {
    brushed_motor_interrupt.set_enc_idle_state(val);
}

extern "C" void gripper_force_stopwatch_overflow_callback_glue(
    uint16_t seconds) {
    if (seconds > 0) {
        brushed_motor_interrupt.stopwatch_overflow(seconds);
    }
}

void grip_motor_iface::initialize() {
    initialize_hardware_g();
    set_brushed_motor_timer_callback(
        call_brushed_motor_handler, gripper_enc_overflow_callback_glue,
        gripper_enc_idle_state_callback_glue,
        gripper_force_stopwatch_overflow_callback_glue);
}

auto grip_motor_iface::get_grip_motor()
    -> brushed_motor::BrushedMotor<lms::GearBoxConfig>& {
    return grip_motor;
}

static auto gmh_tsk = motor_hardware_task::MotorHardwareTask{
    &brushed_motor_hardware_iface, "grip motor hardware task"};
auto grip_motor_iface::get_grip_motor_hardware_task()
    -> motor_hardware_task::MotorHardwareTask& {
    return gmh_tsk;
}
