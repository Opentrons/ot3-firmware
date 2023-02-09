/*
 * firmware-specific functions, data, and hooks for host comms control
 */
#include "rear-panel/firmware/freertos_comms_task.hpp"

#include <array>
#include <functional>
#include <iterator>
#include <utility>

#include "FreeRTOS.h"
#include "common/core/freertos_message_queue.hpp"
#include "rear-panel/core/double_buffer.hpp"
#include "rear-panel/core/messages.hpp"
#include "rear-panel/core/tasks.hpp"
#include "rear-panel/core/tasks/host_comms_task.hpp"
#include "rear-panel/firmware/usb_hardware.h"
#include "task.h"

/** Sadly this must be manually duplicated from usbd_cdc.h */
constexpr size_t CDC_BUFFER_SIZE = 512U;

struct CommsTaskFreeRTOS {
    double_buffer::DoubleBuffer<uint8_t, CDC_BUFFER_SIZE * 4> rx_buf;
    double_buffer::DoubleBuffer<uint8_t, CDC_BUFFER_SIZE * 4> tx_buf;
    uint8_t *committed_rx_buf_ptr;
};

static auto cdc_init_handler() -> uint8_t *;
static auto cdc_deinit_handler() -> void;
// NOLINTNEXTLINE(readability-named-parameter)
static auto cdc_rx_handler(uint8_t *, uint32_t *) -> uint8_t *;

namespace host_comms_control_task {

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
static freertos_message_queue::FreeRTOSMessageQueue<
    messages::HostCommTaskMessage>
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
    _comms_queue("Comms Message Queue");
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
static CommsTaskFreeRTOS _local_task = {
    .rx_buf = {}, .tx_buf = {}, .committed_rx_buf_ptr = nullptr};

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
static auto _top_task = host_comms_task::HostCommTask(_comms_queue);
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
static auto _tasks = std::make_pair(&_top_task, &_local_task);

static constexpr uint32_t stack_size = 2048;
// Stack as a std::array because why not
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
static std::array<StackType_t, stack_size> stack;

// Internal FreeRTOS data structure
static StaticTask_t
    data;  // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

// Actual function that runs in the task
void run(void *param) {  // NOLINT(misc-unused-parameters)
    auto *task_pair = static_cast<decltype(_tasks) *>(param);
    auto *local_task = task_pair->second;
    auto *top_task = task_pair->first;

    usb_hw_init(&cdc_rx_handler, &cdc_init_handler, &cdc_deinit_handler);
    usb_hw_start();
    local_task->committed_rx_buf_ptr = local_task->rx_buf.committed()->data();
    while (true) {
        auto *tx_end =
            top_task->run_once(local_task->tx_buf.accessible()->begin(),
                               local_task->tx_buf.accessible()->end());
        // if (!top_task->may_connect()) {
        //    usb_hw_stop();
        //} else
        if (tx_end != local_task->tx_buf.accessible()->data()) {
            local_task->tx_buf.swap();
            usb_hw_send(local_task->tx_buf.committed()->data(),
                        tx_end - local_task->tx_buf.committed()->data());
        }
    }
}

// Function that creates and spins up the task
auto start() -> rear_panel_tasks::Task<
    TaskHandle_t, host_comms_task::HostCommTask<
                      freertos_message_queue::FreeRTOSMessageQueue>> {
    auto *handle = xTaskCreateStatic(run, "HostCommsControl", stack.size(),
                                     &_tasks, 1, stack.data(), &data);
    return rear_panel_tasks::Task<TaskHandle_t, decltype(_top_task)>{
        .handle = handle, .task = &_top_task};
}
}  // namespace host_comms_control_task

static auto cdc_init_handler() -> uint8_t * {
    using namespace host_comms_control_task;
    _local_task.committed_rx_buf_ptr = _local_task.rx_buf.committed()->data();
    return _local_task.committed_rx_buf_ptr;
}

static auto cdc_deinit_handler() -> void {
    using namespace host_comms_control_task;
    _local_task.committed_rx_buf_ptr = _local_task.rx_buf.committed()->data();
}

// these casting helper functions make it possible to send an element from one variant
// to a variant type that is a super set of that variant. If it is not a super set this will not compile
template <class... Args>
struct variant_cast_proxy
{
    std::variant<Args...> v;

    // extracted into helper function
    template <class... ToArgs>
    static constexpr bool is_convertible() noexcept {
        return (std::is_convertible_v<Args, std::variant<ToArgs...>> && ...);
    }

    template<class... ToArgs, std::enable_if_t<is_convertible<ToArgs...>(), int> = 0>
    operator std::variant<ToArgs...>() const
    {
         return std::visit(
            [](auto&& arg) -> std::variant<ToArgs...> { 
                if constexpr (std::is_convertible_v<decltype(arg), std::variant<ToArgs...>>)
                    return arg;
            },
            v
        );
    }
};

template <class... Args>
auto variant_cast(const std::variant<Args...>& v) -> variant_cast_proxy<Args...>
{
    return { v };
}

/*
** CDC_Receive is a callback hook invoked from the CDC class internals in an
**
** interrupt context. Buf points to the pre-provided rx buf, into which the data
** from the hardware-isolated USB packet memory area has been copied; Len is a
** pointer to the length of data.
**
** - We always start after a buffer swap with the beginning of the committed
**   buffer
** - When we receive a message we swap the buffers
**     and  send the one that just got swapped out to the task for parsing
*/

// NOLINTNEXTLINE(readability-non-const-parameter)
static auto cdc_rx_handler(uint8_t *Buf, uint32_t *Len) -> uint8_t * {
    using namespace host_comms_control_task;
    uint16_t type = 0;
    std::ignore =
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        bit_utils::bytes_to_int(Buf, Buf + sizeof(messages::MessageType), type);
    auto message = messages::rear_panel_parser.parse(
        messages::MessageType(type), _local_task.rx_buf.committed()->data(),
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        Buf + *Len);
    // if parse didn't return anything it means it was malformed so send an ack_failed
    if (message.index() == 0) {
        static_cast<void>(_top_task.get_queue().try_write_isr(messages::AckFailed{.length = 0}));
    } else {
        static_cast<void>(_top_task.get_queue().try_write_isr(variant_cast(message)));
    }
    _local_task.rx_buf.swap();
    _local_task.committed_rx_buf_ptr = _local_task.rx_buf.committed()->data();
    return _local_task.committed_rx_buf_ptr;
}
