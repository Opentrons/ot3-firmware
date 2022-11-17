#include "gripper/core/interfaces.hpp"

#include "can/simlib/transport.hpp"
#include "gripper/core/tasks.hpp"
#include "gripper/simulation/sim_interfaces.hpp"
#include "motor-control/core/brushed_motor/brushed_motor_interrupt_handler.hpp"
#include "motor-control/core/stepper_motor/motor_interrupt_handler.hpp"
#include "motor-control/core/stepper_motor/tmc2130.hpp"
#include "motor-control/simulation/brushed_motor_interrupt_driver.hpp"
#include "motor-control/simulation/motor_interrupt_driver.hpp"
#include "motor-control/simulation/sim_motor_driver_hardware_iface.hpp"
#include "motor-control/simulation/sim_motor_hardware_iface.hpp"
#include "spi/simulation/spi.hpp"

/**
 * The SPI bus.
 */
static auto spi_comms = spi::hardware::SimSpiDeviceBase();

/**
 * The motor interface.
 */
static auto motor_interface =
    sim_motor_hardware_iface::SimMotorHardwareIface(MoveMessageHardware::z_g);

/**
 * The pending move queue
 */
static freertos_message_queue::FreeRTOSMessageQueue<motor_messages::Move>
    motor_queue("Motor Queue");

/**
 * The pending brushed move queue
 */
static freertos_message_queue::FreeRTOSMessageQueue<motor_messages::BrushedMove>
    brushed_motor_queue("Brushed Motor Queue");
/**
 * Motor driver configuration.
 */
static tmc2130::configs::TMC2130DriverConfig MotorDriverConfigurations{
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

static auto z_motor_sys_config =
    lms::LinearMotionSystemConfig<lms::LeadScrewConfig>{
        .mech_config = lms::LeadScrewConfig{.lead_screw_pitch = 4},
        .steps_per_rev = 200,
        .microstep = 16,
        .encoder_pulses_per_rev = 0};
/**
 * The motor struct.
 */
static motor_class::Motor motor{
    z_motor_sys_config, motor_interface,
    motor_messages::MotionConstraints{.min_velocity = 1,
                                      .max_velocity = 2,
                                      .min_acceleration = 1,
                                      .max_acceleration = 2},
    motor_queue};

/**
 * Handler of motor interrupts.
 */
static motor_handler::MotorInterruptHandler motor_interrupt(
    motor_queue, gripper_tasks::z_tasks::get_queues(), motor_interface);

static motor_interrupt_driver::MotorInterruptDriver A(motor_queue,
                                                      motor_interrupt,
                                                      motor_interface);

/**
 * Brushed motor components
 */
static auto brushed_motor_driver_iface =
    sim_brushed_motor_hardware_iface::SimBrushedMotorDriverIface();

static auto brushed_motor_hardware_iface =
    sim_motor_hardware_iface::SimBrushedMotorHardwareIface(
        MoveMessageHardware::g);

static auto gear_conf = lms::LinearMotionSystemConfig<lms::GearBoxConfig>{
    .mech_config = lms::GearBoxConfig{.gear_diameter = 9},
    .steps_per_rev = 0,
    .microstep = 0,
    .encoder_pulses_per_rev = 512,
    .gear_ratio = 84.29};

static auto grip_motor = brushed_motor::BrushedMotor(
    gear_conf, brushed_motor_hardware_iface, brushed_motor_driver_iface,
    brushed_motor_queue);

static brushed_motor_handler::BrushedMotorInterruptHandler
    brushed_motor_interrupt(brushed_motor_queue,
                            gripper_tasks::g_tasks::get_queues(),
                            brushed_motor_hardware_iface,
                            brushed_motor_driver_iface, gear_conf);

static brushed_motor_interrupt_driver::BrushedMotorInterruptDriver G(
    brushed_motor_queue, brushed_motor_interrupt, brushed_motor_hardware_iface);

void z_motor_iface::initialize() {
    motor_interface.provide_mech_config(z_motor_sys_config);
};

void grip_motor_iface::initialize(){};

auto z_motor_iface::get_spi() -> spi::hardware::SpiDeviceBase& {
    return spi_comms;
}

auto z_motor_iface::get_z_motor() -> motor_class::Motor<lms::LeadScrewConfig>& {
    return motor;
}

auto grip_motor_iface::get_grip_motor()
    -> brushed_motor::BrushedMotor<lms::GearBoxConfig>& {
    return grip_motor;
}

auto z_motor_iface::get_tmc2130_driver_configs()
    -> tmc2130::configs::TMC2130DriverConfig& {
    return MotorDriverConfigurations;
}

auto z_motor_iface::get_z_motor_interface()
    -> sim_motor_hardware_iface::SimMotorHardwareIface& {
    return motor_interface;
}

auto z_motor_iface::get_brushed_motor_interface()
    -> sim_motor_hardware_iface::SimBrushedMotorHardwareIface& {
    return brushed_motor_hardware_iface;
}
