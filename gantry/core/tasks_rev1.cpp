#include "gantry/core/tasks_rev1.hpp"

#include "gantry/core/can_task.hpp"
#include "gantry/core/queues.hpp"
#include "gantry/core/utils.hpp"
#include "gantry/firmware/eeprom_keys.hpp"
#include "motor-control/core/tasks/motion_controller_task.hpp"
#include "motor-control/core/tasks/motor_hardware_task.hpp"
#include "motor-control/core/tasks/move_group_task.hpp"
#include "motor-control/core/tasks/move_status_reporter_task.hpp"
#include "motor-control/core/tasks/tmc2160_motor_driver_task.hpp"
#include "spi/core/tasks/spi_task.hpp"
#include "spi/core/writer.hpp"

static auto tasks = gantry::tasks::AllTask{};
static auto queues = gantry::queues::QueueClient{utils::get_node_id()};

static auto spi_task_client =
    spi::writer::Writer<freertos_message_queue::FreeRTOSMessageQueue>();

static auto mc_task_builder =
    freertos_task::TaskStarter<512,
                               motion_controller_task::MotionControllerTask>{};
static auto motor_driver_task_builder =
    freertos_task::TaskStarter<512, tmc2160::tasks::MotorDriverTask>{};
static auto move_group_task_builder =
    freertos_task::TaskStarter<512, move_group_task::MoveGroupTask>{};
static auto move_status_task_builder = freertos_task::TaskStarter<
    512, move_status_reporter_task::MoveStatusReporterTask>{};

static auto spi_task_builder =
    freertos_task::TaskStarter<512, spi::tasks::Task>{};

template <template <typename> typename QueueImpl>
using PollerWithTimer =
    i2c::tasks::I2CPollerTask<QueueImpl, freertos_timer::FreeRTOSTimer>;
static auto i2c2_task_client =
    i2c::writer::Writer<freertos_message_queue::FreeRTOSMessageQueue>();
static auto i2c2_task_builder =
    freertos_task::TaskStarter<512, i2c::tasks::I2CTask>{};
static auto i2c2_poll_task_builder =
    freertos_task::TaskStarter<1024, PollerWithTimer>{};
static auto i2c2_poll_client =
    i2c::poller::Poller<freertos_message_queue::FreeRTOSMessageQueue>{};
static auto eeprom_task_builder =
    freertos_task::TaskStarter<512, eeprom::task::EEPromTask>{};

static auto usage_storage_task_builder =
    freertos_task::TaskStarter<512, usage_storage_task::UsageStorageTask>{};
static auto eeprom_data_rev_update_builder =
    freertos_task::TaskStarter<512, eeprom::data_rev_task::UpdateDataRevTask>{};

static auto tail_accessor = eeprom::dev_data::DevDataTailAccessor{queues};
/**
 * Start gantry ::tasks.
 */
auto gantry::tasks::start_tasks(
    can::bus::CanBus& can_bus,
    motion_controller::MotionController<lms::BeltConfig>& motion_controller,
    spi::hardware::SpiDeviceBase& spi_device,
    tmc2160::configs::TMC2160DriverConfig& driver_configs,
    motor_hardware_task::MotorHardwareTask& mh_tsk,
    i2c::hardware::I2CBase& i2c2,
    eeprom::hardware_iface::EEPromHardwareIface& eeprom_hw_iface) -> interfaces::diag0_handler {
    auto& can_writer = can_task::start_writer(can_bus);
    can_task::start_reader(can_bus);
    auto& motion = mc_task_builder.start(5, "motion controller",
                                         motion_controller, ::queues, ::queues, ::queues);
    auto& tmc2160_driver = motor_driver_task_builder.start(
        5, "tmc2160 driver", driver_configs, ::queues, spi_task_client);
    auto& move_group =
        move_group_task_builder.start(5, "move group", ::queues, ::queues);
    auto& move_status_reporter = move_status_task_builder.start(
        5, "move status", ::queues, motion_controller.get_mechanical_config(),
        ::queues);

    auto& spi_task = spi_task_builder.start(5, "spi task", spi_device);
    spi_task_client.set_queue(&spi_task.get_queue());

    auto& i2c2_task = i2c2_task_builder.start(5, "i2c2", i2c2);
    i2c2_task_client.set_queue(&i2c2_task.get_queue());
    auto& i2c2_poller_task =
        i2c2_poll_task_builder.start(5, "i2c2 poller", i2c2_task_client);
    i2c2_poll_client.set_queue(&i2c2_poller_task.get_queue());

    auto& eeprom_task = eeprom_task_builder.start(5, "eeprom", i2c2_task_client,
                                                  eeprom_hw_iface);
    auto& usage_storage_task = usage_storage_task_builder.start(
        5, "usage storage", ::queues, ::queues, tail_accessor);

    auto& eeprom_data_rev_update_task = eeprom_data_rev_update_builder.start(
        5, "data_rev_update", ::queues, tail_accessor, table_updater);

    ::tasks.can_writer = &can_writer;
    ::tasks.motion_controller = &motion;
    ::tasks.tmc2160_driver = &tmc2160_driver;
    ::tasks.move_group = &move_group;
    ::tasks.move_status_reporter = &move_status_reporter;
    ::tasks.spi_task = &spi_task;
    ::tasks.i2c2_task = &i2c2_task;
    ::tasks.i2c2_poller_task = &i2c2_poller_task;
    ::tasks.eeprom_task = &eeprom_task;
    ::tasks.usage_storage_task = &usage_storage_task;
    ::tasks.update_data_rev_task = &eeprom_data_rev_update_task;

    ::queues.motion_queue = &motion.get_queue();
    ::queues.motor_driver_queue = &tmc2160_driver.get_queue();
    ::queues.move_group_queue = &move_group.get_queue();
    ::queues.set_queue(&can_writer.get_queue());
    ::queues.move_status_report_queue = &move_status_reporter.get_queue();
    ::queues.spi_queue = &spi_task.get_queue();
    ::queues.i2c2_queue = &i2c2_task.get_queue();
    ::queues.i2c2_poller_queue = &i2c2_poller_task.get_queue();
    ::queues.eeprom_queue = &eeprom_task.get_queue();
    ::queues.usage_storage_queue = &usage_storage_task.get_queue();

    mh_tsk.start_task();

    return gantry::tasks::call_run_diag0_interrupt;
}

void gantry::tasks::call_run_diag0_interrupt() {
    if (gantry::tasks::get_tasks().motion_controller) {
        return gantry::tasks::get_tasks().motion_controller->run_diag0_interrupt();
    }
}

gantry::queues::QueueClient::QueueClient(can::ids::NodeId this_fw)
    : can::message_writer::MessageWriter{this_fw} {}

void gantry::queues::QueueClient::send_motion_controller_queue(
    const motion_controller_task::TaskMessage& m) {
    motion_queue->try_write(m);
}

void gantry::queues::QueueClient::send_motor_driver_queue(
    const tmc2160::tasks::TaskMessage& m) {
    motor_driver_queue->try_write(m);
}

void gantry::queues::QueueClient::send_move_group_queue(
    const move_group_task::TaskMessage& m) {
    move_group_queue->try_write(m);
}

void gantry::queues::QueueClient::send_move_status_reporter_queue(
    const move_status_reporter_task::TaskMessage& m) {
    static_cast<void>(move_status_report_queue->try_write_isr(m));
}

void gantry::queues::QueueClient::send_eeprom_queue(
    const eeprom::task::TaskMessage& m) {
    eeprom_queue->try_write(m);
}

void gantry::queues::QueueClient::send_usage_storage_queue(
    const usage_storage_task::TaskMessage& m) {
    usage_storage_queue->try_write(m);
}
/**
 * Access to the tasks singleton
 * @return
 */
auto gantry::tasks::get_tasks() -> AllTask& { return ::tasks; }

/**
 * Access to the queues singleton
 * @return
 */
auto gantry::queues::get_queues() -> QueueClient& { return ::queues; }
