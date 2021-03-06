// clang-format off
#include "FreeRTOS.h"
#include "system_stm32g4xx.h"
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
#include "mount_detection.hpp"
#include "pipettes/core/central_tasks.hpp"
#include "pipettes/core/gear_motor_tasks.hpp"
#include "pipettes/core/linear_motor_tasks.hpp"
#include "pipettes/core/motor_configurations.hpp"
#include "pipettes/core/peripheral_tasks.hpp"
#include "pipettes/core/pipette_type.h"
#include "pipettes/core/sensor_tasks.hpp"
#include "pipettes/firmware/interfaces.hpp"
#include "sensors/firmware/sensor_hardware.hpp"
#include "spi/firmware/spi_comms.hpp"

#pragma GCC diagnostic push
// NOLINTNEXTLINE(clang-diagnostic-unknown-warning-option)
#pragma GCC diagnostic ignored "-Wvolatile"
//#include "motor_encoder_hardware.h"
#include "motor_hardware.h"
//#include "motor_timer_hardware.h"
#include "pipettes/firmware/i2c_setup.h"
#pragma GCC diagnostic pop

constexpr auto PIPETTE_TYPE = get_pipette_type();

static auto iWatchdog = iwdg::IndependentWatchDog{};

static auto can_bus_1 = can::hal::bus::HalCanBus(
    can_get_device_handle(),
    // TODO we need to modify this based on pipette type as well since
    // LED pin changes from board to board
    gpio::PinConfig{// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                    .port = GPIOB,
                    .pin = GPIO_PIN_11,
                    .active_setting = GPIO_PIN_RESET});

spi::hardware::SPI_interface SPI_intf = {.SPI_handle = &hspi2};

static spi::hardware::Spi spi_comms(SPI_intf);

static auto i2c_comms3 = i2c::hardware::I2C();
static auto i2c_comms2 = i2c::hardware::I2C();
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

static auto motor_config = motor_configs::motor_configurations<PIPETTE_TYPE>();

static auto interrupt_queues = interfaces::get_interrupt_queues<PIPETTE_TYPE>();

static auto linear_motor_hardware =
    interfaces::linear_motor::get_motor_hardware(motor_config.hardware_pins);
static auto plunger_interrupt = interfaces::linear_motor::get_interrupt(
    linear_motor_hardware, interrupt_queues);
static auto linear_motion_control =
    interfaces::linear_motor::get_motion_control(linear_motor_hardware,
                                                 interrupt_queues);

static auto gear_hardware =
    interfaces::gear_motor::get_motor_hardware(motor_config.hardware_pins);
static auto gear_interrupts =
    interfaces::gear_motor::get_interrupts(gear_hardware, interrupt_queues);
static auto gear_motion_control =
    interfaces::gear_motor::get_motion_control(gear_hardware, interrupt_queues);

extern "C" void plunger_callback() { plunger_interrupt.run_interrupt(); }

extern "C" void gear_callback_wrapper() {
    interfaces::gear_motor::gear_callback(gear_interrupts);
}

static auto pins_for_sensor =
    motor_configs::sensor_configurations<PIPETTE_TYPE>();

auto sensor_hardware =
    sensors::hardware::SensorHardware(pins_for_sensor.primary);

extern "C" void HAL_GPIO_EXTI_Falling_Callback(uint16_t GPIO_Pin) {
    if (GPIO_Pin == pins_for_sensor.primary.data_ready.pin) {
        sensor_hardware.data_ready();
    }
}

// Unfortunately, these numbers need to be literals or defines
// to get the compile-time checks to work so we can't actually
// correctly rely on the hal to get these numbers - they need
// to be checked against current configuration. However, they are
// - clock input is 170MHz assuming the CAN is clocked from sysclk
// - 100ns time quantum
// - 500khz bitrate
// - 80% sample point
// Should drive
// segment 1 = 15 quanta
// segment 2 = 4 quantum

// For the exact timing values these generate see
// can/tests/test_bit_timings.cpp
static constexpr auto can_bit_timings =
    can::bit_timings::BitTimings<170 * can::bit_timings::MHZ, 100,
                                 500 * can::bit_timings::KHZ, 800>{};

auto initialize_motor_tasks(
    can::ids::NodeId id,
    motor_configs::HighThroughputPipetteDriverHardware& conf,
    interfaces::gear_motor::GearMotionControl& gear_motion) {
    sensor_tasks::start_tasks(*central_tasks::get_tasks().can_writer,
                              peripheral_tasks::get_i2c3_client(),
                              peripheral_tasks::get_i2c1_client(),
                              peripheral_tasks::get_i2c1_poller_client(),
                              sensor_hardware, id, eeprom_hardware_iface);

    initialize_linear_timer(plunger_callback);
    initialize_gear_timer(gear_callback_wrapper);
    linear_motor_tasks::start_tasks(
        *central_tasks::get_tasks().can_writer, linear_motion_control,
        peripheral_tasks::get_spi_client(), conf.linear_motor, id);
    gear_motor_tasks::start_tasks(*central_tasks::get_tasks().can_writer,
                                  gear_motion,
                                  peripheral_tasks::get_spi_client(), conf, id);
}
auto initialize_motor_tasks(
    can::ids::NodeId id,
    motor_configs::LowThroughputPipetteDriverHardware& conf,
    interfaces::gear_motor::UnavailableGearMotionControl&) {
    sensor_tasks::start_tasks(*central_tasks::get_tasks().can_writer,
                              peripheral_tasks::get_i2c3_client(),
                              peripheral_tasks::get_i2c1_client(),
                              peripheral_tasks::get_i2c3_poller_client(),
                              sensor_hardware, id, eeprom_hardware_iface);

    initialize_linear_timer(plunger_callback);
    linear_motor_tasks::start_tasks(
        *central_tasks::get_tasks().can_writer, linear_motion_control,
        peripheral_tasks::get_spi_client(), conf.linear_motor, id);
}

auto main() -> int {
    HardwareInit();
    RCC_Peripheral_Clock_Select();
    utility_gpio_init();
    adc_init();
    initialize_enc(PIPETTE_TYPE);
    auto id = pipette_mounts::detect_id();

    i2c_setup(&i2chandler_struct);
    i2c_comms3.set_handle(i2chandler_struct.i2c3);
    i2c_comms2.set_handle(i2chandler_struct.i2c2);

    if (initialize_spi() != HAL_OK) {
        Error_Handler();
    }

    app_update_clear_flags();

    can_bus_1.start(can_bit_timings);

    central_tasks::start_tasks(can_bus_1, id);
    peripheral_tasks::start_tasks(i2c_comms3, i2c_comms2, spi_comms);
    initialize_motor_tasks(id, motor_config.driver_configs,
                           gear_motion_control);

    iWatchdog.start(6);

    vTaskStartScheduler();
}
