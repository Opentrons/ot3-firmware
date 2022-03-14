#include <signal.h>

#include <cstdlib>
#include <cstring>

#include "FreeRTOS.h"
#include "can/core/ids.hpp"
#include "can/simlib/sim_canbus.hpp"
#include "can/simlib/transport.hpp"
#include "common/core/freertos_message_queue.hpp"
#include "common/core/logging.h"
#include "common/simulation/i2c_sim.hpp"
#include "common/simulation/spi.hpp"
#include "motor-control/core/motor.hpp"
#include "motor-control/core/tmc2130.hpp"
#include "motor-control/simulation/motor_interrupt_driver.hpp"
#include "motor-control/simulation/sim_motor_hardware_iface.hpp"
#include "pipettes/core/tasks.hpp"
#include "sensors/simulation/eeprom.hpp"
#include "sensors/simulation/fdc1004.hpp"
#include "sensors/simulation/hdc2080.hpp"
#include "sensors/simulation/sensors.hpp"
#include "task.h"

static auto can_bus_1 = sim_canbus::SimCANBus{can_transport::create()};

static freertos_message_queue::FreeRTOSMessageQueue<motor_messages::Move>
    motor_queue("Motor Queue");

static sim_spi::SimSpiDeviceBase spi_comms{};

static sim_motor_hardware_iface::SimMotorHardwareIface plunger_hw{};

static motor_handler::MotorInterruptHandler plunger_interrupt(
    motor_queue, pipettes_tasks::get_queues(), plunger_hw);

static motor_interrupt_driver::MotorInterruptDriver sim_interrupt(
    motor_queue, plunger_interrupt);

// microstepping is currently set to 32 μsteps.
static tmc2130::TMC2130DriverConfig MotorDriverConfigurations{
    .registers = {.gconfig = {.en_pwm_mode = 1},
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
                               .mres = 0x3},
                  .coolconf = {.sgt = 0x6}},
    .current_config = {
        .r_sense = 0.1,
        .v_sf = 0.325,
    }};

static auto hdcsensor = hdc2080_simulator::HDC2080{};
static auto capsensor = fdc1004_simulator::FDC1004{};
static auto eeprom = eeprom_simulator::EEProm{};
std::map<uint16_t, sensor_simulator::SensorType> sensor_map = {
    {hdcsensor.ADDRESS, hdcsensor},
    {eeprom.ADDRESS, eeprom},
    {capsensor.ADDRESS, capsensor}};

static auto i2c_comms = sim_i2c::SimI2C{sensor_map};

static motor_class::Motor pipette_motor{
    spi_comms,
    lms::LinearMotionSystemConfig<lms::LeadScrewConfig>{
        .mech_config = lms::LeadScrewConfig{.lead_screw_pitch = 3.03},
        .steps_per_rev = 200,
        .microstep = 32},
    plunger_hw,
    motor_messages::MotionConstraints{.min_velocity = 1,
                                      .max_velocity = 2,
                                      .min_acceleration = 1,
                                      .max_acceleration = 2},
    MotorDriverConfigurations,
    motor_queue};

static auto node_from_env(const char* env) -> can_ids::NodeId {
    if (!env) {
        LOG("On left mount by default");
        return can_ids::NodeId::pipette_left;
    }
    if (strncmp(env, "left", strlen("left")) == 0) {
        LOG("On left mount from env var");
        return can_ids::NodeId::pipette_left;
    } else if (strncmp(env, "right", strlen("right")) == 0) {
        LOG("On right mount from env var");
        return can_ids::NodeId::pipette_right;
    } else {
        LOG("On left mount from invalid env var");
        return can_ids::NodeId::pipette_left;
    }
}

void signal_handler(int signum) {
    LOG("Interrupt signal (%d) received.", signum);
    exit(signum);
}

int main() {
    signal(SIGINT, signal_handler);

    LOG_INIT("PIPETTE", []() -> const char * {return pcTaskGetName(xTaskGetCurrentTaskHandle());});

    pipettes_tasks::start_tasks(can_bus_1, pipette_motor.motion_controller,
                                pipette_motor.driver, i2c_comms,
                                node_from_env(std::getenv("MOUNT")));
    vTaskStartScheduler();
}
