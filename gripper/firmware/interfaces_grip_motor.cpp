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
    .encoder_interrupt_freq =
        double(GRIPPER_JAW_PWM_FREQ_HZ) / double(GRIPPER_JAW_PWM_WIDTH),
    .pid_kd = 0.05, .pid_ki = 0, .pid_kd = 0,
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
        .vref = 0.5, .pwm_min = 7, .pwm_max = 100},
    update_pwm);

static lms::LinearMotionSystemConfig<lms::GearBoxConfig> gear_config {
    .mech_config = lms::GearBoxConfig{.gear_diameter = 9}, .steps_per_rev = 0,
    .microstep = 0, .encoder_pulses_per_rev = 512, .gear_ratio = 84.29
};

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
