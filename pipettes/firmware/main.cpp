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
#include "common/firmware/iwdg.hpp"
#include "common/firmware/utility_gpio.h"
#include "eeprom/core/hardware_iface.hpp"

// todo check if needed
#include "i2c/firmware/i2c_comms.hpp"
#include "motor-control/core/stepper_motor/motor_encoder_background_timer.hpp"
#include "pipettes/core/central_tasks.hpp"
#include "pipettes/core/configs.hpp"
#include "pipettes/core/gear_motor_tasks.hpp"
#include "pipettes/core/linear_motor_tasks.hpp"
#include "pipettes/core/motor_configurations.hpp"
#include "pipettes/core/mount_sense.hpp"
#include "pipettes/core/peripheral_tasks.hpp"
#include "pipettes/core/pipette_type.h"
#include "pipettes/core/sensor_tasks.hpp"
#include "pipettes/firmware/interfaces_g4.hpp"
#include "pipettes/firmware/utility_configurations.hpp"
#include "sensors/firmware/sensor_hardware.hpp"
#include "spi/firmware/spi_comms.hpp"

#pragma GCC diagnostic push
// NOLINTNEXTLINE(clang-diagnostic-unknown-warning-option)
#pragma GCC diagnostic ignored "-Wvolatile"
#include "motor_hardware.h"
#include "pipette_utility_gpio.h"
#include "pipettes/firmware/i2c_setup.h"
#pragma GCC diagnostic pop

constexpr auto PIPETTE_TYPE = get_pipette_type();

static auto iWatchdog = iwdg::IndependentWatchDog{};

static interfaces::linear_motor::diag0_handler call_diag0_handler = nullptr;

static auto can_bus_1 = can::hal::bus::HalCanBus(
    can_get_device_handle(), utility_configs::led_gpio(PIPETTE_TYPE));

spi::hardware::SPI_interface SPI_intf = {.SPI_handle = &hspi2};

static spi::hardware::Spi spi_comms(SPI_intf);

static auto i2c_comms3 = i2c::hardware::I2C();
static auto i2c_comms2 = i2c::hardware::I2C();
static I2CHandlerStruct i2chandler_struct{};

static auto eeprom_chip =
    PIPETTE_TYPE == NINETY_SIX_CHANNEL
        ? eeprom::hardware_iface::EEPromChipType::ST_M24128_DF
        : eeprom::hardware_iface::EEPromChipType::ST_M24128_BF;

class PipetteEEPromHardwareIface
    : public eeprom::hardware_iface::EEPromHardwareIface {
  public:
    PipetteEEPromHardwareIface()
        : eeprom::hardware_iface::EEPromHardwareIface(eeprom_chip) {}
    void set_write_protect(bool enable) final {
        if (enable) {
            disable_eeprom_write();
        } else {
            enable_eeprom_write();
        }
    }
};

static auto eeprom_hardware_iface = PipetteEEPromHardwareIface{};

static auto linear_stall_check = stall_check::StallCheck(
    configs::linear_motion_sys_config_by_axis(PIPETTE_TYPE)
            .get_encoder_pulses_per_mm() /
        1000.0F,
    configs::linear_motion_sys_config_by_axis(PIPETTE_TYPE)
            .get_usteps_per_mm() /
        1000.0F,
    configs::STALL_THRESHOLD_UM);

// Gear motors have no encoders
static auto gear_stall_check = interfaces::gear_motor::GearStallCheck{
    .left = stall_check::StallCheck(0, 0, 0),
    .right = stall_check::StallCheck(0, 0, 0)};

static auto motor_config = motor_configs::motor_configurations<PIPETTE_TYPE>();

static auto interrupt_queues = interfaces::get_interrupt_queues<PIPETTE_TYPE>();

static auto linear_motor_hardware =
    interfaces::linear_motor::get_motor_hardware(
        motor_config.hardware_pins.linear_motor);
static auto plunger_interrupt = interfaces::linear_motor::get_interrupt(
    linear_motor_hardware, interrupt_queues, linear_stall_check);
static auto linear_motion_control =
    interfaces::linear_motor::get_motion_control(linear_motor_hardware,
                                                 interrupt_queues);

static auto encoder_background_timer =
    motor_encoder::BackgroundTimer(plunger_interrupt, linear_motor_hardware);

static auto gear_hardware =
    interfaces::gear_motor::get_motor_hardware(motor_config.hardware_pins);
static auto gear_interrupts = interfaces::gear_motor::get_interrupts(
    gear_hardware, interrupt_queues, gear_stall_check);
static auto gear_motion_control =
    interfaces::gear_motor::get_motion_control(gear_hardware, interrupt_queues);

extern "C" void plunger_callback() { plunger_interrupt.run_interrupt(); }

extern "C" void gear_callback_wrapper() {
    interfaces::gear_motor::gear_callback(gear_interrupts);
}

void encoder_callback(int32_t direction) {
    interfaces::linear_motor::encoder_interrupt(linear_motor_hardware,
                                                direction);
}

static auto pins_for_sensor =
    utility_configs::sensor_configurations<PIPETTE_TYPE>();
static auto sensor_hardware_container =
    utility_configs::get_sensor_hardware_container(pins_for_sensor);

static auto tip_sense_gpio_primary = pins_for_sensor.primary.tip_sense.value();

static auto& sensor_queue_client = sensor_tasks::get_queues();

static auto tail_accessor =
    eeprom::dev_data::DevDataTailAccessor{sensor_queue_client};

extern "C" void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    if (GPIO_Pin == tip_sense_gpio_primary.pin) {
        if (sensor_queue_client.tip_notification_queue_rear != nullptr) {
            static_cast<void>(
                sensor_queue_client.tip_notification_queue_rear->try_write_isr(
                    sensors::tip_presence::TipStatusChangeDetected{}));
        }
    } else if (GPIO_Pin == linear_motor_hardware.get_pins().diag0.pin) {
        if (call_diag0_handler != nullptr) {
            if (*call_diag0_handler != nullptr) {
                (*call_diag0_handler)();
            }
        }
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

static auto lmh_tsk = motor_hardware_task::MotorHardwareTask{
    &linear_motor_hardware, "linear motor hardware task"};
static auto gmh_tsks =
    interfaces::gear_motor::get_motor_hardware_tasks(gear_hardware);

auto initialize_motor_tasks(
    can::ids::NodeId id,
    motor_configs::HighThroughputPipetteDriverHardware& conf,
    interfaces::gear_motor::GearMotionControl& gear_motion,
    motor_hardware_task::MotorHardwareTask& lmh_tsk,
    interfaces::gear_motor::GearMotorHardwareTasks& gmh_tsks) {
    sensor_tasks::start_tasks(*central_tasks::get_tasks().can_writer,
                              peripheral_tasks::get_i2c3_client(),
                              peripheral_tasks::get_i2c3_poller_client(),
                              peripheral_tasks::get_i2c1_client(),
                              peripheral_tasks::get_i2c1_poller_client(),
                              sensor_hardware_container.primary,
                              sensor_hardware_container.secondary.value(), id,
                              eeprom_hardware_iface);

    initialize_linear_timer(plunger_callback);
    initialize_gear_timer(gear_callback_wrapper);
    initialize_enc_timer(encoder_callback);
    call_diag0_handler = linear_motor_tasks::start_tasks(
        *central_tasks::get_tasks().can_writer, linear_motion_control,
        peripheral_tasks::get_spi_client(), conf.linear_motor, id, lmh_tsk,
        tail_accessor);
    gear_motor_tasks::start_tasks(
        *central_tasks::get_tasks().can_writer, gear_motion,
        peripheral_tasks::get_spi_client(), conf, id, gmh_tsks, tail_accessor);
}
auto initialize_motor_tasks(
    can::ids::NodeId id,
    motor_configs::LowThroughputPipetteDriverHardware& conf,
    interfaces::gear_motor::UnavailableGearMotionControl&,
    motor_hardware_task::MotorHardwareTask& lmh_tsk,
    interfaces::gear_motor::UnavailableGearHardwareTasks&) {
    if (PIPETTE_TYPE == EIGHT_CHANNEL) {
        sensor_tasks::start_tasks(*central_tasks::get_tasks().can_writer,
                                  peripheral_tasks::get_i2c3_client(),
                                  peripheral_tasks::get_i2c3_poller_client(),
                                  peripheral_tasks::get_i2c1_client(),
                                  peripheral_tasks::get_i2c1_poller_client(),
                                  sensor_hardware_container.primary,
                                  sensor_hardware_container.secondary.value(),
                                  id, eeprom_hardware_iface);
    } else {
        sensor_tasks::start_tasks(*central_tasks::get_tasks().can_writer,
                                  peripheral_tasks::get_i2c3_client(),
                                  peripheral_tasks::get_i2c3_poller_client(),
                                  peripheral_tasks::get_i2c1_client(),
                                  peripheral_tasks::get_i2c1_poller_client(),
                                  sensor_hardware_container.primary, id,
                                  eeprom_hardware_iface);
    }

    initialize_linear_timer(plunger_callback);
    initialize_enc_timer(encoder_callback);
    call_diag0_handler = linear_motor_tasks::start_tasks(
        *central_tasks::get_tasks().can_writer, linear_motion_control,
        peripheral_tasks::get_spi_client(), conf.linear_motor, id, lmh_tsk,
        tail_accessor);
}

auto main() -> int {
    HardwareInit();
    RCC_Peripheral_Clock_Select();
    utility_gpio_init();

    /*
    ** This delay exists after we set the mount ID pin to an input and before
    ** we read it. The delay must exist and be long to cover the following
    ** situations:
    ** - When transitioning from the bootloader, which has already done the
    **   mount ID process and driven the line, we need to release the line and
    **   give it time to reestablish its intended voltage
    ** - When the robot is turning on while we're plugged in, we need to give
    *the
    **   head board time to start up and establish the voltage before we try and
    **   read it
    **
    ** Don't change this value without testing these scenarios in addition to
    ** hotplugging the pipette into a running robot.
    */
    delay_start(100);
    auto id =
        pipette_mounts::decide_id(utility_gpio_get_mount_id(PIPETTE_TYPE) == 1);

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
    initialize_motor_tasks(id, motor_config.driver_configs, gear_motion_control,
                           lmh_tsk, gmh_tsks);
    encoder_background_timer.start();
    iWatchdog.start(6);

    vTaskStartScheduler();
}
