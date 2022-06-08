#include "pipettes/core/peripheral_tasks.hpp"

#include "common/core/freertos_task.hpp"

static auto tasks = peripheral_tasks::Tasks{};
static auto queue_client = peripheral_tasks::QueueClient{};

static auto i2c1_task _client =
    i2c::writer::Writer<freertos_message_queue::FreeRTOSMessageQueue>();
static auto i2c3_task_client =
    i2c::writer::Writer<freertos_message_queue::FreeRTOSMessageQueue>();

static auto i2c1_task_builder =
    freertos_task::TaskStarter<512, i2c::tasks::I2CTask>{};
static auto i2c3_task_builder =
    freertos_task::TaskStarter<512, i2c::tasks::I2CTask>{};
template <template <typename> typename QueueImpl>
using PollerWithTimer =
    i2c::tasks::I2CPollerTask<QueueImpl, freertos_timer::FreeRTOSTimer>;

static auto i2c1_poll_task_builder =
    freertos_task::TaskStarter<1024, PollerWithTimer>{};
static auto i2c3_poll_task_builder =
    freertos_task::TaskStarter<1024, PollerWithTimer>{};
static auto i2c1_poll_client =
    i2c::poller::Poller<freertos_message_queue::FreeRTOSMessageQueue>{};
static auto i2c3_poll_client =
    i2c::poller::Poller<freertos_message_queue::FreeRTOSMessageQueue>{};

static auto spi_task_client =
    spi::writer::Writer<freertos_message_queue::FreeRTOSMessageQueue>();

static auto spi_task_builder =
    freertos_task::TaskStarter<512, spi::tasks::Task>{};

void peripheral_tasks::start_tasks(i2c::hardware::I2CBase& i2c3_interface,
                                   i2c::hardware::I2CBase& i2c1_interface,
                                   spi::hardware::SpiDeviceBase& spi_device) {
    auto& queues = peripheral_tasks::get_queues();
    auto& tasks = peripheral_tasks::get_tasks();

    auto& i2c3_task = i2c3_task_builder.start(5, "i2c3", i2c3_interface);
    i2c3_task_client.set_queue(&i2c3_task.get_queue());
    auto& i2c1_task = i2c1_task_builder.start(5, "i2c1", i2c1_interface);
    i2c1_task_client.set_queue(&i2c1_task.get_queue());

    auto& i2c3_poller_task =
        i2c3_poll_task_builder.start(5, "i2c3 poller", i2c3_task_client);
    auto& i2c1_poller_task =
        i2c1_poll_task_builder.start(5, "i2c1 poller", i2c1_task_client);
    i2c1_poll_client.set_queue(&i2c1_poller_task.get_queue());
    i2c3_poll_client.set_queue(&i2c3_poller_task.get_queue());

    auto& spi_task = spi_task_builder.start(5, "spi task", spi_device);
    spi_task_client.set_queue(&spi_task.get_queue());

    tasks.i2c3_task = &i2c3_task;
    tasks.i2c1_task = &i2c1_task;
    tasks.i2c3_poller_task = &i2c3_poller_task;
    tasks.i2c1_poller_task = &i2c1_poller_task;
    tasks.spi_task = &spi_task;

    queues.i2c3_queue = &i2c3_task.get_queue();
    queues.i2c1_queue = &i2c1_task.get_queue();
    queues.i2c3_poller_queue = &i2c3_poller_task.get_queue();
    queues.i2c1_poller_queue = &i2c1_poller_task.get_queue();
    queues.spi_queue = &spi_task.get_queue();
}

peripheral_tasks::QueueClient::QueueClient() {}

auto peripheral_tasks::get_tasks() -> Tasks& { return tasks; }

auto peripheral_tasks::get_queues() -> QueueClient& { return queue_client; }

auto peripheral_tasks::get_i2c3_client() -> peripheral_tasks::I2CClient& {
    return i2c3_task_client;
}

auto peripheral_tasks::get_i2c1_client() -> peripheral_tasks::I2CClient& {
    return i2c1_task_client;
}

auto peripheral_tasks::get_i2c1_poller_client()
    -> i2c::poller::Poller<freertos_message_queue::FreeRTOSMessageQueue>& {
    return i2c1_poll_client;
}

auto peripheral_tasks::get_i2c3_poller_client()
    -> i2c::poller::Poller<freertos_message_queue::FreeRTOSMessageQueue>& {
    return i2c3_poll_client;
}

auto peripheral_tasks::get_spi_client() -> peripheral_tasks::SPIClient& {
    return spi_task_client;
}
