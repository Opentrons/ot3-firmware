#include "gripper/core/can_task.hpp"
#include "gripper/core/interfaces.hpp"
#include "motor-control/core/brushed_motor/brushed_motor.hpp"
#include "motor-control/core/brushed_motor/brushed_motor_interrupt_handler.hpp"
#include "motor-control/firmware/brushed_motor/brushed_motor_hardware.hpp"
#include "motor-control/firmware/brushed_motor/driver_hardware.hpp"
#pragma GCC diagnostic push
// NOLINTNEXTLINE(clang-diagnostic-unknown-warning-option)
#pragma GCC diagnostic ignored "-Wvolatile"
#include "motor_encoder_hardware.h"
#include "motor_hardware.h"

#pragma GCC diagnostic pop

constexpr uint32_t PWM_MAX = 100;
constexpr uint32_t PWM_MIN = 7;

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
            .port = GPIOC,
            .pin = GPIO_PIN_11,
            .active_setting = GPIO_PIN_SET},
    .limit_switch =
        {  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
            .port = GPIOC,
            .pin = GPIO_PIN_2,
            .active_setting = GPIO_PIN_SET},
    .encoder_interrupt_freq = double(GRIPPER_JAW_PWM_FREQ_HZ),

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
    brushed_motor_conf, &htim2);

/**
 * The brushed motor driver hardware interface.
 */
static brushed_motor_driver::BrushedMotorDriver brushed_motor_driver_iface(
    dac_config,
    brushed_motor_driver::DriverConfig{
        .vref = 1, .pwm_min = PWM_MIN, .pwm_max = PWM_MAX},
    update_pwm);

static lms::LinearMotionSystemConfig<lms::GearBoxConfig> gear_config{
    .mech_config = lms::GearBoxConfig{.gear_diameter = 9},
    .steps_per_rev = 0,
    .microstep = 0,
    .encoder_pulses_per_rev = 512,
    .gear_ratio = 103.81};

static brushed_motor::BrushedMotor grip_motor(gear_config,
                                              brushed_motor_hardware_iface,
                                              brushed_motor_driver_iface,
                                              motor_queue);

/**
 * Handler of brushed motor interrupts.
 */
static brushed_motor_handler::BrushedMotorInterruptHandler
    brushed_motor_interrupt(motor_queue, gripper_tasks::g_tasks::get_queues(),
                            brushed_motor_hardware_iface,
                            brushed_motor_driver_iface, gear_config);

extern "C" void call_brushed_motor_handler(void) {
    brushed_motor_interrupt.run_interrupt();
}

extern "C" void gripper_enc_overflow_callback_glue(int32_t direction) {
    brushed_motor_hardware_iface.encoder_overflow(direction);
}

extern "C" void gripper_enc_idle_state_callback_glue(bool val) {
    brushed_motor_interrupt.set_enc_idle_state(val);
}

void grip_motor_iface::initialize() {
    // Initialize DAC
    initialize_dac();
    initialize_enc();
    set_brushed_motor_timer_callback(call_brushed_motor_handler,
                                     gripper_enc_overflow_callback_glue,
                                     gripper_enc_idle_state_callback_glue);
}

auto grip_motor_iface::get_grip_motor()
    -> brushed_motor::BrushedMotor<lms::GearBoxConfig>& {
    return grip_motor;
}
