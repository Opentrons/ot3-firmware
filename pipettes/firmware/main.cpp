// clang-format off
#include "FreeRTOS.h"
#include "system_stm32l5xx.h"
#include "task.h"

// clang-format on

#include "can/core/bit_timings.hpp"
#include "can/core/ids.hpp"
#include "can/firmware/hal_can_bus.hpp"
#include "common/core/app_update.h"
#include "common/firmware/clocking.h"
#include "common/firmware/errors.h"
#include "common/firmware/gpio.hpp"
#include "common/firmware/iwdg.hpp"
#include "common/firmware/utility_gpio.h"
#include "eeprom/core/hardware_iface.hpp"

// todo check if needed
#include "i2c/firmware/i2c_comms.hpp"
#include "motor-control/core/linear_motion_system.hpp"
#include "motor-control/core/motor_messages.hpp"
#include "motor-control/core/stepper_motor/motion_controller.hpp"
#include "motor-control/core/stepper_motor/motor_interrupt_handler.hpp"
#include "motor-control/firmware/stepper_motor/pipette_motor_hardware.hpp"
#include "mount_detection.hpp"
#include "pipettes/core/central_tasks.hpp"
#include "pipettes/core/configs.hpp"
#include "pipettes/core/gear_motor_tasks.hpp"
#include "pipettes/core/interfaces.hpp"
#include "pipettes/core/linear_motor_tasks.hpp"
#include "pipettes/core/peripheral_tasks.hpp"
#include "pipettes/core/pipette_type.h"
#include "pipettes/core/sensor_tasks.hpp"
#include "sensors/firmware/sensor_hardware.hpp"
#include "spi/firmware/spi_comms.hpp"

#pragma GCC diagnostic push
// NOLINTNEXTLINE(clang-diagnostic-unknown-warning-option)
#pragma GCC diagnostic ignored "-Wvolatile"
#include "motor_encoder_hardware.h"
#include "motor_hardware.h"
#include "motor_timer_hardware.h"
#include "pipettes/firmware/i2c_setup.h"
#pragma GCC diagnostic pop

constexpr auto PIPETTE_TYPE = get_pipette_type();

static auto iWatchdog = iwdg::IndependentWatchDog{};

static auto can_bus_1 = hal_can_bus::HalCanBus(
    can_get_device_handle(),
    gpio::PinConfig{// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                    .port = GPIOA,
                    .pin = GPIO_PIN_8,
                    .active_setting = GPIO_PIN_RESET});

static freertos_message_queue::FreeRTOSMessageQueue<motor_messages::Move>
    linear_motor_queue("Linear Motor Queue");
static freertos_message_queue::FreeRTOSMessageQueue<motor_messages::Move>
    right_gear_motor_queue("Right Gear Motor Queue");
static freertos_message_queue::FreeRTOSMessageQueue<motor_messages::Move>
    left_gear_motor_queue("Left Gear Motor Queue");

spi::hardware::SPI_interface SPI_intf = {.SPI_handle = &hspi2};

static spi::hardware::Spi spi_comms(SPI_intf);

static auto i2c_comms3 = i2c::hardware::I2C();
static auto i2c_comms1 = i2c::hardware::I2C();
static I2CHandlerStruct i2chandler_struct{};

class EEPromHardwareIface : public eeprom::hardware_iface::EEPromHardwareIface {
  public:
    void set_write_protect(bool enable) final {
        if (enable) {
            disable_eeprom_write();
        } else {
            enable_eeprom_write();
        }
    }
};
static auto eeprom_hardware_iface = EEPromHardwareIface();

static auto motor_config = interfaces::motor_configurations<PIPETTE_TYPE>();

static pipette_motor_hardware::MotorHardware plunger_hw(
    motor_config.hardware_pins.linear_motor, &htim7, &htim2);

static motor_handler::MotorInterruptHandler plunger_interrupt(
    linear_motor_queue, linear_motor_tasks::get_queues(), plunger_hw);

// TODO put this in a function
static motor_handler::MotorInterruptHandler gear_interrupt_left(
    left_gear_motor_queue, gear_motor_tasks::get_queues(), left_gear_hw);
static motor_handler::MotorInterruptHandler gear_interrupt_right(
    right_gear_motor_queue, gear_motor_tasks::get_queues(), right_gear_hw);


// TODO pull out of main
static motion_controller::MotionController plunger_motion_control{
    configs::linear_motion_sys_config_by_axis(PIPETTE_TYPE), plunger_hw,
    motor_messages::MotionConstraints{.min_velocity = 1,
                                      .max_velocity = 2,
                                      .min_acceleration = 1,
                                      .max_acceleration = 2},
    linear_motor_queue};
static pipette_motion_controller::PipetteMotionController left_gear_motion_control{
    configs::linear_motion_sys_config_by_axis(PIPETTE_TYPE), left_gear_hw,
    motor_messages::MotionConstraints{.min_velocity = 1,
        .max_velocity = 2,
        .min_acceleration = 1,
        .max_acceleration = 2},
    left_gear_motor_queue};

static pipette_motion_controller::PipetteMotionController right_gear_motion_control{
    configs::linear_motion_sys_config_by_axis(PIPETTE_TYPE), right_gear_hw,
    motor_messages::MotionConstraints{.min_velocity = 1,
        .max_velocity = 2,
        .min_acceleration = 1,
        .max_acceleration = 2},
    right_gear_motor_queue};

extern "C" void plunger_callback() { plunger_interrupt.run_interrupt(); }

extern "C" void gear_callback() {
    gear_interrupt_left.run_interrupt();
    gear_interrupt_right.run_interrupt();
}

static auto pins_for_sensor = interfaces::sensor_configurations<PIPETTE_TYPE>();

auto sensor_hardware =
    sensors::hardware::SensorHardware(pins_for_sensor.primary);

// Unfortunately, these numbers need to be literals or defines
// to get the compile-time checks to work so we can't actually
// correctly rely on the hal to get these numbers - they need
// to be checked against current configuration. However, they are
// - clock input is 100MHz assuming the CAN is clocked from sysclk
// - 100ns time quantum
// - 500khz bitrate
// - 80% sample point
// Should drive
// segment 1 = 15 quanta
// segment 2 = 4 quantum

// For the exact timing values these generate see
// can/tests/test_bit_timings.cpp
static constexpr auto can_bit_timings =
    can::bit_timings::BitTimings<100 * can::bit_timings::MHZ, 100,
                                 500 * can::bit_timings::KHZ, 800>{};

auto initialize_motor_tasks(
    can_ids::NodeId id, interfaces::HighThroughputPipetteDriverHardware& conf) {
    sensor_tasks::start_tasks(*central_tasks::get_tasks().can_writer,
                              peripheral_tasks::get_i2c3_client(),
                              peripheral_tasks::get_i2c1_client(),
                              peripheral_tasks::get_i2c1_poller_client(),
                              sensor_hardware, id, eeprom_hardware_iface);

    initialize_linear_timer(plunger_callback);
    initialize_gear_timer(gear_callback);
    linear_motor_tasks::start_tasks(
        *central_tasks::get_tasks().can_writer, pipette_motor.motion_controller,
        peripheral_tasks::get_spi_client(), conf.linear_motor, id);
    // todo update with correct motion controller.
    gear_motor_tasks::start_tasks(
        *central_tasks::get_tasks().can_writer, pipette_motor.motion_controller,
        peripheral_tasks::get_spi_client(), conf.right_gear_motor, id);
}
auto initialize_motor_tasks(
    can_ids::NodeId id, interfaces::LowThroughputPipetteDriverHardware& conf) {
    sensor_tasks::start_tasks(*central_tasks::get_tasks().can_writer,
                              peripheral_tasks::get_i2c3_client(),
                              peripheral_tasks::get_i2c1_client(),
                              peripheral_tasks::get_i2c1_poller_client(),
                              sensor_hardware, id, eeprom_hardware_iface);

    initialize_linear_timer(plunger_callback);
    linear_motor_tasks::start_tasks(
        *central_tasks::get_tasks().can_writer, pipette_motor.motion_controller,
        peripheral_tasks::get_spi_client(), conf.linear_motor, id);
}

auto main() -> int {
    HardwareInit();
    RCC_Peripheral_Clock_Select();
    MX_ICACHE_Init();
    utility_gpio_init();
    adc_init();
    initialize_enc(PIPETTE_TYPE);
    auto id = pipette_mounts::detect_id();

    i2c_setup(&i2chandler_struct);
    i2c_comms3.set_handle(i2chandler_struct.i2c3);
    i2c_comms1.set_handle(i2chandler_struct.i2c1);

    if (initialize_spi() != HAL_OK) {
        Error_Handler();
    }

    app_update_clear_flags();

    can_bus_1.start(can_bit_timings);

    central_tasks::start_tasks(can_bus_1, id);
    peripheral_tasks::start_tasks(i2c_comms3, i2c_comms1, spi_comms);
    initialize_motor_tasks(id, motor_config.driver_configs);

    iWatchdog.start(6);

    vTaskStartScheduler();
}