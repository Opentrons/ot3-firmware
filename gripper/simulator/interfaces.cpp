#include "gripper/core/interfaces.hpp"

#include "can/simlib/sim_canbus.hpp"
#include "can/simlib/transport.hpp"
#include "common/simulation/spi.hpp"
#include "gripper/core/tasks.hpp"
#include "motor-control/core/motor_interrupt_handler.hpp"
#include "motor-control/simulation/motor_interrupt_driver.hpp"
#include "motor-control/simulation/sim_motor_hardware_iface.hpp"

/**
 * The CAN bus.
 */
static auto canbus = sim_canbus::SimCANBus(can_transport::create());

/**
 * The SPI bus.
 */
static auto spi_comms = sim_spi::SimSpiDeviceBase();

/**
 * The motor interface.
 */
static auto motor_interface = sim_motor_hardware_iface::SimMotorHardwareIface();

/**
 * The pending move queue
 */
static freertos_message_queue::FreeRTOSMessageQueue<motor_messages::Move>
    motor_queue("Motor Queue");

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
 * The motor struct.
 */
static motor_class::Motor motor{
    spi_comms,
    lms::LinearMotionSystemConfig<lms::LeadScrewConfig>{
        .mech_config = lms::LeadScrewConfig{.lead_screw_pitch = 4},
        .steps_per_rev = 200,
        .microstep = 16},
    motor_interface,
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
    motor_queue, gripper_tasks::get_queues(), motor_interface);

static motor_interrupt_driver::MotorInterruptDriver A(motor_queue,
                                                      motor_interrupt);

void interfaces::initialize() {}

auto interfaces::get_can_bus() -> can_bus::CanBus& { return canbus; }

auto interfaces::get_spi() -> spi::SpiDeviceBase& { return spi_comms; }

auto interfaces::get_motor_hardware_iface()
    -> motor_hardware::MotorHardwareIface& {
    return motor_interface;
}

auto interfaces::get_motor() -> motor_class::Motor<lms::LeadScrewConfig>& {
    return motor;
}