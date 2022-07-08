#include "can/simlib/sim_canbus.hpp"
#include "can/simlib/transport.hpp"
#include "gantry/core/interfaces_proto.hpp"
#include "gantry/core/queues.hpp"
#include "gantry/core/utils.hpp"
#include "motor-control/core/stepper_motor/motor_interrupt_handler.hpp"
#include "motor-control/simulation/motor_interrupt_driver.hpp"
#include "motor-control/simulation/sim_motor_hardware_iface.hpp"
#include "spi/simulation/spi.hpp"

/**
 * The CAN bus.
 */
static auto canbus = can::sim::bus::SimCANBus(can::sim::transport::create());

/**
 * The SPI bus.
 */
static auto spi_comms = spi::hardware::SimSpiDeviceBase();

/**
 * The motor interface.
 */
static auto motor_interface = sim_motor_hardware_iface::SimMotorHardwareIface();

/**
 * The pending move queue
 */
static freertos_message_queue::FreeRTOSMessageQueue<motor_messages::Move>
    motor_queue("Motor Queue");

static tmc2130::configs::TMC2130DriverConfig driver_configs{
    .registers = {.gconfig = {.en_pwm_mode = 1},
                  .ihold_irun = {.hold_current = 0x2,
                                 .run_current = 0x18,
                                 .hold_current_delay = 0x7},
                  .thigh = {.threshold = 0xFFFFF},
                  .chopconf = {.toff = 0x5,
                               .hstrt = 0x5,
                               .hend = 0x3,
                               .tbl = 0x2,
                               .mres = 0x3},
                  .coolconf = {.sgt = 0x6}},
    .current_config =
        {
            .r_sense = 0.1,
            .v_sf = 0.325,
        },
    .chip_select{
        .cs_pin = 0,
        .GPIO_handle = 0,
    }};

/**
 * The motor struct.
 */
static motor_class::Motor motor{
    lms::LinearMotionSystemConfig<lms::BeltConfig>{
        .mech_config = lms::BeltConfig{.pulley_diameter = 12.7},
        .steps_per_rev = 200,
        .microstep = 32,
        .encoder_ppr = 1000},
    motor_interface,
    motor_messages::MotionConstraints{.min_velocity = 1,
                                      .max_velocity = 2,
                                      .min_acceleration = 1,
                                      .max_acceleration = 2},
    motor_queue};

/**
 * Handler of motor interrupts.
 */
static motor_handler::MotorInterruptHandler motor_interrupt(
    motor_queue, gantry::queues::get_queues(), motor_interface);

static motor_interrupt_driver::MotorInterruptDriver A(motor_queue,
                                                      motor_interrupt,
                                                      motor_interface);

void interfaces::initialize() {}

auto interfaces::get_can_bus() -> can::bus::CanBus& { return canbus; }

auto interfaces::get_spi() -> spi::hardware::SpiDeviceBase& {
    return spi_comms;
}

auto interfaces::get_motor_hardware_iface()
    -> motor_hardware::StepperMotorHardwareIface& {
    return motor_interface;
}

auto interfaces::get_motor() -> motor_class::Motor<lms::BeltConfig>& {
    return motor;
}

auto interfaces::get_driver_config() -> tmc2130::configs::TMC2130DriverConfig& {
    return driver_configs;
}
