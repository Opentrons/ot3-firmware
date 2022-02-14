#include <signal.h>

#include "FreeRTOS.h"
#include "can/simlib/sim_canbus.hpp"
#include "common/core/logging.h"
#include "common/simulation/spi.hpp"
#include "head/core/presence_sensing_driver.hpp"
#include "head/core/tasks.hpp"
#include "head/simulation/adc.hpp"
#include "motor-control/core/motor.hpp"
#include "motor-control/core/motor_interrupt_handler.hpp"
#include "motor-control/core/tmc2130.hpp"
#include "motor-control/simulation/motor_interrupt_driver.hpp"
#include "motor-control/simulation/sim_motor_hardware_iface.hpp"
#include "task.h"

/**
 * The CAN bus.
 */
static auto canbus = sim_canbus::SimCANBus(can_transport::create());

/**
 * The SPI busses.
 */
static auto spi_comms_right = sim_spi::SimSpiDeviceBase();
static auto spi_comms_left = sim_spi::SimSpiDeviceBase();

/**
 * The motor interfaces.
 */
static auto motor_interface_right =
    sim_motor_hardware_iface::SimMotorHardwareIface();
static auto motor_interface_left =
    sim_motor_hardware_iface::SimMotorHardwareIface();

static freertos_message_queue::FreeRTOSMessageQueue<motor_messages::Move>
    motor_queue_right("Motor Queue Right");
static freertos_message_queue::FreeRTOSMessageQueue<motor_messages::Move>
    motor_queue_left("Motor Queue Left");

static tmc2130::TMC2130DriverConfig MotorDriverConfigurations{
    .registers =
        {
            .gconfig = {.en_pwm_mode = 1},
            .ihold_irun = {.hold_current = 0x2,
                           .run_current = 0x10,
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
    .current_config =
        {
            .r_sense = 0.1,
            .v_sf = 0.325,
        },
};

static motor_handler::MotorInterruptHandler motor_interrupt_right(
    motor_queue_right, head_tasks::get_right_queues(), motor_interface_right);

static motor_class::Motor motor_right{
    spi_comms_right,
    lms::LinearMotionSystemConfig<lms::LeadScrewConfig>{
        .mech_config = lms::LeadScrewConfig{.lead_screw_pitch = 12},
        .steps_per_rev = 200,
        .microstep = 16},
    motor_interface_right,
    motor_messages::MotionConstraints{.min_velocity = 1,
                                      .max_velocity = 2,
                                      .min_acceleration = 1,
                                      .max_acceleration = 2},
    MotorDriverConfigurations,
    motor_queue_right};

static motor_handler::MotorInterruptHandler motor_interrupt_left(
    motor_queue_left, head_tasks::get_left_queues(), motor_interface_left);

static motor_class::Motor motor_left{
    spi_comms_left,
    lms::LinearMotionSystemConfig<lms::LeadScrewConfig>{
        .mech_config = lms::LeadScrewConfig{.lead_screw_pitch = 12},
        .steps_per_rev = 200,
        .microstep = 16},
    motor_interface_left,
    motor_messages::MotionConstraints{.min_velocity = 1,
                                      .max_velocity = 2,
                                      .min_acceleration = 1,
                                      .max_acceleration = 2},
    MotorDriverConfigurations,
    motor_queue_left};

static motor_interrupt_driver::MotorInterruptDriver sim_interrupt_right(
    motor_queue_right, motor_interrupt_right);
static motor_interrupt_driver::MotorInterruptDriver sim_interrupt_left(
    motor_queue_left, motor_interrupt_left);

static auto adc_comms = adc::SimADC{};

static auto presence_sense_driver =
    presence_sensing_driver::PresenceSensingDriver(adc_comms);

void signal_handler(int signum) {
    LOG("Interrupt signal (%d) received.\n", signum);
    exit(signum);
}

int main() {
    signal(SIGINT, signal_handler);

    head_tasks::start_tasks(canbus, motor_left.motion_controller,
                            motor_left.driver, motor_right.motion_controller,
                            motor_right.driver, presence_sense_driver);

    vTaskStartScheduler();
    return 0;
}
