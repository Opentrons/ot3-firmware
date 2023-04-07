#pragma once

#include "can/core/can_writer_task.hpp"
#include "can/core/ids.hpp"
#include "can/core/message_writer.hpp"
#include "eeprom/core/hardware_iface.hpp"
#include "eeprom/core/task.hpp"
#include "i2c/core/hardware_iface.hpp"
#include "i2c/core/tasks/i2c_poller_task.hpp"
#include "i2c/core/tasks/i2c_task.hpp"
#include "i2c/core/writer.hpp"
#include "motor-control/core/linear_motion_system.hpp"
#include "motor-control/core/tasks/motion_controller_task.hpp"
#include "motor-control/core/tasks/move_group_task.hpp"
#include "motor-control/core/tasks/move_status_reporter_task.hpp"
#include "motor-control/core/tasks/tmc_motor_driver_common.hpp"
#include "motor-control/core/tasks/usage_storage_task.hpp"
#include "spi/core/spi.hpp"
#include "spi/core/tasks/spi_task.hpp"

namespace gantry {
namespace queues {

/**
 * Access to all the message queues in the system.
 */
struct QueueClient : can::message_writer::MessageWriter {
    QueueClient(can::ids::NodeId this_fw);

    void send_motion_controller_queue(
        const motion_controller_task::TaskMessage& m);

    void send_motor_driver_queue(const tmc::tasks::TaskMessage& m);

    void send_move_group_queue(const move_group_task::TaskMessage& m);

    void send_move_status_reporter_queue(
        const move_status_reporter_task::TaskMessage& m);

    void send_eeprom_queue(const eeprom::task::TaskMessage& m);

    void send_usage_storage_queue(const usage_storage_task::TaskMessage& m);

    freertos_message_queue::FreeRTOSMessageQueue<
        motion_controller_task::TaskMessage>* motion_queue{nullptr};
    freertos_message_queue::FreeRTOSMessageQueue<tmc::tasks::TaskMessage>*
        motor_driver_queue{nullptr};
    freertos_message_queue::FreeRTOSMessageQueue<move_group_task::TaskMessage>*
        move_group_queue{nullptr};
    freertos_message_queue::FreeRTOSMessageQueue<
        move_status_reporter_task::TaskMessage>* move_status_report_queue{
        nullptr};
    freertos_message_queue::FreeRTOSMessageQueue<spi::tasks::TaskMessage>*
        spi_queue{nullptr};
    freertos_message_queue::FreeRTOSMessageQueue<i2c::writer::TaskMessage>*
        i2c2_queue{nullptr};
    freertos_message_queue::FreeRTOSMessageQueue<i2c::poller::TaskMessage>*
        i2c2_poller_queue{nullptr};
    freertos_message_queue::FreeRTOSMessageQueue<eeprom::task::TaskMessage>*
        eeprom_queue{nullptr};
    freertos_message_queue::FreeRTOSMessageQueue<
        usage_storage_task::TaskMessage>* usage_storage_queue{nullptr};
};

/**
 * Access to the queues singleton
 * @return
 */
[[nodiscard]] auto get_queues() -> QueueClient&;
};  // namespace queues
};  // namespace gantry
