#include "pipettes/core/head_peripheral_tasks.hpp"

#include "common/core/freertos_task.hpp"

static auto tasks = head_peripheral_tasks::Tasks{};
static auto queue_client = head_peripheral_tasks::QueueClient{};

static auto i2c3_task_client =
    i2c::writer::Writer<freertos_message_queue::FreeRTOSMessageQueue>();

static auto i2c3_task_builder =
    freertos_task::TaskStarter<512, i2c::tasks::I2CTask>{};
template <template <typename> typename QueueImpl>
using PollerWithTimer =
    i2c::tasks::I2CPollerTask<QueueImpl, freertos_timer::FreeRTOSTimer>;

static auto i2c3_poll_task_builder =
    freertos_task::TaskStarter<1024, PollerWithTimer>{};
static auto i2c3_poll_client =
    i2c::poller::Poller<freertos_message_queue::FreeRTOSMessageQueue>{};

static auto spi2_task_client =
    spi::writer::Writer<freertos_message_queue::FreeRTOSMessageQueue>();

static auto spi2_task_builder =
    freertos_task::TaskStarter<512, spi::tasks::Task>{};

static auto spi3_task_client =
    spi::writer::Writer<freertos_message_queue::FreeRTOSMessageQueue>();

static auto spi3_task_builder =
    freertos_task::TaskStarter<512, spi::tasks::Task>{};

void head_peripheral_tasks::start_tasks(
    i2c::hardware::I2CBase& i2c3_interface,
    spi::hardware::SpiDeviceBase& spi2_device,
    spi::hardware::SpiDeviceBase& spi3_device) {
    auto& queues = head_peripheral_tasks::get_queues();
    auto& tasks = head_peripheral_tasks::get_tasks();

    auto& i2c3_task = i2c3_task_builder.start(5, "i2c3", i2c3_interface);
    i2c3_task_client.set_queue(&i2c3_task.get_queue());

    auto& i2c3_poller_task =
        i2c3_poll_task_builder.start(5, "i2c3 poller", i2c3_task_client);

    i2c3_poll_client.set_queue(&i2c3_poller_task.get_queue());

    auto& spi2_task = spi2_task_builder.start(5, "spi task", spi2_device);
    spi2_task_client.set_queue(&spi2_task.get_queue());

    auto& spi3_task = spi2_task_builder.start(5, "spi task", spi3_device);
    spi3_task_client.set_queue(&spi3_task.get_queue());

    tasks.i2c3_task = &i2c3_task;
    tasks.i2c3_poller_task = &i2c3_poller_task;
    tasks.spi2_task = &spi2_task;
    tasks.spi3_task = &spi3_task;

    queues.i2c3_queue = &i2c3_task.get_queue();
    queues.i2c3_poller_queue = &i2c3_poller_task.get_queue();
    queues.spi2_queue = &spi2_task.get_queue();
    queues.spi3_queue = &spi3_task.get_queue();
}

head_peripheral_tasks::QueueClient::QueueClient() {}

auto head_peripheral_tasks::get_tasks() -> Tasks& { return tasks; }

auto head_peripheral_tasks::get_queues() -> QueueClient& {
    return queue_client;
}

auto head_peripheral_tasks::get_i2c3_client()
    -> head_peripheral_tasks::I2CClient& {
    return i2c3_task_client;
}

auto head_peripheral_tasks::get_i2c3_poller_client()
    -> i2c::poller::Poller<freertos_message_queue::FreeRTOSMessageQueue>& {
    return i2c3_poll_client;
}

auto head_peripheral_tasks::get_spi2_client()
    -> head_peripheral_tasks::SPIClient& {
    return spi2_task_client;
}

auto head_peripheral_tasks::get_spi3_client()
    -> head_peripheral_tasks::SPIClient& {
    return spi3_task_client;
}
