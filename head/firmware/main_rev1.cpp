#include <array>
#include <cstdio>
#include <cstring>
#include <tuple>

// clang-format off
#include "FreeRTOS.h"
#include "task.h"
#include "system_stm32g4xx.h"
#include "common/firmware/errors.h"
#include "common/core/app_update.h"
#include "common/firmware/iwdg.hpp"
#include "head/firmware/i2c_setup.h"
// clang-format on
#pragma GCC diagnostic push
// NOLINTNEXTLINE(clang-diagnostic-unknown-warning-option)
#pragma GCC diagnostic ignored "-Wvolatile"
#include "motor_hardware.h"
#include "stm32g4xx_hal.h"
#include "stm32g4xx_hal_conf.h"
#include "utility_hardware.h"
#pragma GCC diagnostic pop
#include "can/core/bit_timings.hpp"
#include "can/firmware/hal_can_bus.hpp"
#include "common/core/freertos_timer.hpp"
#include "common/firmware/clocking.h"
#include "common/firmware/gpio.hpp"
#include "head/core/queues.hpp"
#include "head/core/tasks_rev1.hpp"
#include "head/core/utils.hpp"
#include "head/firmware/eeprom_keys.hpp"
#include "head/firmware/presence_sensing_hardware.hpp"
#include "i2c/firmware/i2c_comms.hpp"
#include "motor-control/core/linear_motion_system.hpp"
#include "motor-control/core/stepper_motor/motor.hpp"
#include "motor-control/core/stepper_motor/motor_encoder_background_timer.hpp"
#include "motor-control/core/stepper_motor/motor_interrupt_handler.hpp"
#include "motor-control/core/stepper_motor/tmc2160.hpp"
#include "motor-control/firmware/stepper_motor/motor_hardware.hpp"
#include "spi/firmware/spi_comms.hpp"

static auto iWatchdog = iwdg::IndependentWatchDog{};

static head_tasks::diag0_handler call_diag0_z_handler = nullptr;
static head_tasks::diag0_handler call_diag0_a_handler = nullptr;

static auto can_bus_1 = can::hal::bus::HalCanBus(
    can_get_device_handle(),
    gpio::PinConfig{// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                    .port = GPIOB,
                    .pin = GPIO_PIN_6,
                    .active_setting = GPIO_PIN_RESET});

static freertos_message_queue::FreeRTOSMessageQueue<motor_messages::Move>
    motor_queue_left("Motor Queue Left");

static freertos_message_queue::FreeRTOSMessageQueue<
    can::messages::UpdateMotorPositionEstimationRequest>
    update_position_queue_left("PQueue Left");

static freertos_message_queue::FreeRTOSMessageQueue<motor_messages::Move>
    motor_queue_right("Motor Queue Right");

static freertos_message_queue::FreeRTOSMessageQueue<
    can::messages::UpdateMotorPositionEstimationRequest>
    update_position_queue_right("PQueue Right");

/**
 * @brief SPI MSP Initialization
 * This function configures SPI for the Z/A axis motors
 * @param hspi: SPI handle pointer
 * @retval None
 */

spi::hardware::SPI_interface SPI_intf2 = {
    .SPI_handle = &hspi2,
};
static spi::hardware::Spi spi_comms2(SPI_intf2);

spi::hardware::SPI_interface SPI_intf3 = {
    .SPI_handle = &hspi3,
};
static spi::hardware::Spi spi_comms3(SPI_intf3);

#if PCBA_PRIMARY_REVISION == 'b'
static constexpr int direction_active_level = GPIO_PIN_RESET;
#else
static constexpr int direction_active_level = GPIO_PIN_SET;
#endif

struct motor_hardware::UsageEEpromConfig left_usage_config {
    std::array<UsageRequestSet, 2> {
        UsageRequestSet{
            .eeprom_key = L_MOTOR_DISTANCE_KEY,
            .type_key =
                uint16_t(can::ids::MotorUsageValueType::linear_motor_distance),
            .length = usage_storage_task::distance_data_usage_len},
            UsageRequestSet {
            .eeprom_key = L_ERROR_COUNT_KEY,
            .type_key =
                uint16_t(can::ids::MotorUsageValueType::total_error_count),
            .length = usage_storage_task::error_count_usage_len
        }
    }
};

struct motor_hardware::UsageEEpromConfig right_usage_config {
    std::array<UsageRequestSet, 2> {
        UsageRequestSet{
            .eeprom_key = R_MOTOR_DISTANCE_KEY,
            .type_key =
                uint16_t(can::ids::MotorUsageValueType::linear_motor_distance),
            .length = usage_storage_task::distance_data_usage_len},
            UsageRequestSet {
            .eeprom_key = R_ERROR_COUNT_KEY,
            .type_key =
                uint16_t(can::ids::MotorUsageValueType::total_error_count),
            .length = usage_storage_task::error_count_usage_len
        }
    }
};

struct motor_hardware::HardwareConfig pin_configurations_left {
    .direction =
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
            .port = GPIOC,
            .pin = GPIO_PIN_1,
            .active_setting = direction_active_level},
    .step =
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
            .port = GPIOC,
            .pin = GPIO_PIN_0,
            .active_setting = GPIO_PIN_SET},
    .enable =
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
            .port = GPIOC,
            .pin = GPIO_PIN_4,
            .active_setting = GPIO_PIN_SET},
    .limit_switch =
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
            .port = GPIOB,
            .pin = GPIO_PIN_7,
            .active_setting = GPIO_PIN_SET},
    .led = {},
    .sync_in =
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
            .port = GPIOA,
            .pin = GPIO_PIN_8,
            .active_setting = GPIO_PIN_RESET},
    .estop_in =
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
            .port = GPIOB,
            .pin = GPIO_PIN_4,
            .active_setting = GPIO_PIN_RESET},
    .diag0 =
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
            .port = GPIOC,
            .pin = GPIO_PIN_13,
            .active_setting = GPIO_PIN_RESET},
    .ebrake = gpio::PinConfig {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
        .port = GPIOB, .pin = GPIO_PIN_5, .active_setting = GPIO_PIN_RESET
    }
};

struct motor_hardware::HardwareConfig pin_configurations_right {
    .direction =
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
            .port = GPIOC,
            .pin = GPIO_PIN_7,
            .active_setting = direction_active_level},
    .step =
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
            .port = GPIOC,
            .pin = GPIO_PIN_6,
            .active_setting = GPIO_PIN_SET},
    .enable =
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
            .port = GPIOB,
            .pin = GPIO_PIN_11,
            .active_setting = GPIO_PIN_SET},
    .limit_switch =
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
            .port = GPIOB,
            .pin = GPIO_PIN_9,
            .active_setting = GPIO_PIN_SET},
    .led = {},
    .sync_in =
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
            .port = GPIOA,
            .pin = GPIO_PIN_8,
            .active_setting = GPIO_PIN_RESET},
    .estop_in =
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
            .port = GPIOB,
            .pin = GPIO_PIN_4,
            .active_setting = GPIO_PIN_RESET},
    .diag0 =
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
            .port = GPIOC,
            .pin = GPIO_PIN_15,
            .active_setting = GPIO_PIN_RESET},
    .ebrake = gpio::PinConfig {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
        .port = GPIOB, .pin = GPIO_PIN_0, .active_setting = GPIO_PIN_RESET
    }
};

// TODO clean up the head main file by using interfaces.
static tmc2160::configs::TMC2160DriverConfig motor_driver_configs_right{
    .registers = {.gconfig = {.en_pwm_mode = 0, .diag0_error = 1},
                  .ihold_irun = {.hold_current = 16,
                                 .run_current = 31,
                                 .hold_current_delay = 0x7},
                  .tcoolthrs = {.threshold = 0},
                  .thigh = {.threshold = 0},
                  .chopconf = {.toff = 0x2,
                               .hstrt = 0x0,
                               .hend = 0x3,
                               .tbl = 0x1,
                               .tpfd = 0x4,
                               .mres = 0x4},
                  .coolconf = {.sgt = 0x6},
                  .glob_scale = {.global_scaler = 0xA7}},
    .current_config =
        {
            .r_sense = 0.1,
            .v_sf = 0.325,
        },
    .chip_select{
        .cs_pin = GPIO_PIN_12,
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
        .GPIO_handle = GPIOB,
    }};

static tmc2160::configs::TMC2160DriverConfig motor_driver_configs_left{
    .registers = {.gconfig = {.en_pwm_mode = 0, .diag0_error = 1},
                  .ihold_irun = {.hold_current = 16,
                                 .run_current = 31,
                                 .hold_current_delay = 0x7},
                  .tcoolthrs = {.threshold = 0},
                  .thigh = {.threshold = 0x0},
                  .chopconf = {.toff = 0x2,
                               .hstrt = 0x0,
                               .hend = 0x3,
                               .tbl = 0x1,
                               .vhighfs = 0,
                               .vhighchm = 0,
                               .tpfd = 0x4,
                               .mres = 0x4},
                  .coolconf = {.sgt = 0x6},
                  .glob_scale = {.global_scaler = 0xA7}},
    .current_config =
        {
            .r_sense = 0.1,
            .v_sf = 0.325,
        },
    .chip_select{
        .cs_pin = GPIO_PIN_4,
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
        .GPIO_handle = GPIOA,
    }};

#if (PCBA_PRIMARY_REVISION == 'a' || PCBA_PRIMARY_REVISION == 'b')
static constexpr float reduction_ratio = 4.0;
#else
static constexpr float reduction_ratio = 3.2;
#endif

static auto linear_config = lms::LinearMotionSystemConfig<lms::LeadScrewConfig>{
    .mech_config =
        lms::LeadScrewConfig{.lead_screw_pitch = 12.0,
                             .gear_reduction_ratio = reduction_ratio},
    .steps_per_rev = 200,
    .microstep = 16,
    .encoder_pulses_per_rev = 1024.0};

static stall_check::StallCheck stallcheck_right(
    linear_config.get_encoder_pulses_per_mm() / 1000.0F,
    linear_config.get_usteps_per_mm() / 1000.0F, utils::STALL_THRESHOLD_UM);

/**
 * TODO: This motor class is only used in motor handler and should be
 * instantiated inside of the MotorHandler class. However, some refactors
 * should be made to avoid a pretty gross template signature.
 */

static motor_hardware::MotorHardware motor_hardware_right(
    pin_configurations_right, &htim7, &htim2, right_usage_config);
static motor_handler::MotorInterruptHandler motor_interrupt_right(
    motor_queue_right, head_tasks::get_right_queues(),
    head_tasks::get_right_queues(), motor_hardware_right, stallcheck_right,
    update_position_queue_right);

static auto encoder_background_timer_right =
    motor_encoder::BackgroundTimer(motor_interrupt_right, motor_hardware_right);

// engaging motors on boot
static motor_class::Motor motor_right{
    linear_config,
    motor_hardware_right,
    motor_messages::MotionConstraints{.min_velocity = 1,
                                      .max_velocity = 2,
                                      .min_acceleration = 1,
                                      .max_acceleration = 2},
    motor_queue_right,
    update_position_queue_right,
    true};

static stall_check::StallCheck stallcheck_left(
    linear_config.get_encoder_pulses_per_mm() / 1000.0F,
    linear_config.get_usteps_per_mm() / 1000.0F, utils::STALL_THRESHOLD_UM);

static motor_hardware::MotorHardware motor_hardware_left(
    pin_configurations_left, &htim7, &htim3, left_usage_config);
static motor_handler::MotorInterruptHandler motor_interrupt_left(
    motor_queue_left, head_tasks::get_left_queues(),
    head_tasks::get_left_queues(), motor_hardware_left, stallcheck_left,
    update_position_queue_left);

static auto encoder_background_timer_left =
    motor_encoder::BackgroundTimer(motor_interrupt_left, motor_hardware_left);

// engaging motors on boot
static motor_class::Motor motor_left{
    linear_config,
    motor_hardware_left,
    motor_messages::MotionConstraints{.min_velocity = 1,
                                      .max_velocity = 2,
                                      .min_acceleration = 1,
                                      .max_acceleration = 2},
    motor_queue_left,
    update_position_queue_left,
    true};

extern "C" void motor_callback_glue() {
    motor_interrupt_left.run_interrupt();
    motor_interrupt_right.run_interrupt();
}

extern "C" void left_enc_overflow_callback_glue(int32_t direction) {
    motor_hardware_left.encoder_overflow(direction);
}

extern "C" void right_enc_overflow_callback_glue(int32_t direction) {
    motor_hardware_right.encoder_overflow(direction);
}

static auto psd = presence_sensing_driver::PresenceSensingHardware{
    gpio::PinConfig{// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                    .port = GPIOC,
                    .pin = GPIO_PIN_5,
                    .active_setting = GPIO_PIN_SET},
    gpio::PinConfig{// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                    .port = GPIOB,
                    .pin = GPIO_PIN_2,
                    .active_setting = GPIO_PIN_RESET},
    gpio::PinConfig{// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
                    .port = GPIOB,
                    .pin = GPIO_PIN_1,
                    .active_setting = GPIO_PIN_SET}};

auto timer_for_notifier = freertos_timer::FreeRTOSTimer(
    "timer for notifier", ([] {
        auto* presence_sensing_task =
            head_tasks::get_tasks().presence_sensing_driver_task;
        if (presence_sensing_task != nullptr) {
            presence_sensing_task->notifier_callback();
        }
    }),
    100);

// Unfortunately, these numbers need to be literals or defines
// to get the compile-time checks to work so we can't actually
// correctly rely on the hal to get these numbers - they need
// to be checked against current configuration. However, they are
// - clock input is 85MHz assuming the CAN is clocked from PCLK1
// which has a clock divider of 2, and the system clock is 170MHZ
// - 50ns time quantum
// - 250KHz bitrate requested yields 250312KHz actual
// - 88.2% sample point
// Should drive
// segment 1 = 73 quanta
// segment 2 = 11 quanta

// For the exact timing values these generate see
// can/tests/test_bit_timings.cpp
static constexpr auto can_bit_timings =
    can::bit_timings::BitTimings<170 * can::bit_timings::MHZ, 100,
                                 500 * can::bit_timings::KHZ, 800>{};

static auto rmh_tsk =
    motor_hardware_task::MotorHardwareTask{&motor_hardware_right, "rmh task"};
static auto lmh_tsk =
    motor_hardware_task::MotorHardwareTask{&motor_hardware_left, "lmh task"};

static auto i2c_comms3 = i2c::hardware::I2C();
static auto i2c_handles = I2CHandlerStruct{};

static constexpr auto eeprom_chip =
    eeprom::hardware_iface::EEPromChipType::ST_M24128_BF;

class EEPromHardwareInterface
    : public eeprom::hardware_iface::EEPromHardwareIface {
  public:
    EEPromHardwareInterface()
        : eeprom::hardware_iface::EEPromHardwareIface(eeprom_chip) {}
    void set_write_protect(bool enable) final {
        if (enable) {
            disable_eeprom_write();
        } else {
            enable_eeprom_write();
        }
    }
};
static auto eeprom_hw_iface = EEPromHardwareInterface();

auto main() -> int {
    HardwareInit();
    RCC_Peripheral_Clock_Select();

    app_update_clear_flags();

    initialize_timer(motor_callback_glue, left_enc_overflow_callback_glue,
                     right_enc_overflow_callback_glue, &call_diag0_z_handler,
                     &call_diag0_a_handler);

    i2c_setup(&i2c_handles);
    i2c_comms3.set_handle(i2c_handles.i2c3);

    if (initialize_spi(&hspi2) != HAL_OK) {
        Error_Handler();
    }
    if (initialize_spi(&hspi3) != HAL_OK) {
        Error_Handler();
    }

    utility_gpio_init();
    can_bus_1.start(can_bit_timings);
    std::tie(call_diag0_z_handler, call_diag0_a_handler) =
        head_tasks::start_tasks(can_bus_1, motor_left.motion_controller,
                                motor_right.motion_controller, psd, spi_comms2,
                                spi_comms3, motor_driver_configs_left,
                                motor_driver_configs_right, rmh_tsk, lmh_tsk,
                                i2c_comms3, eeprom_hw_iface);

    timer_for_notifier.start();

    encoder_background_timer_left.start();
    encoder_background_timer_right.start();

    iWatchdog.start(6);

    vTaskStartScheduler();
}
