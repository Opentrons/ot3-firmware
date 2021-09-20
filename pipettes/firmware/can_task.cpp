#include "common/firmware/can_task.hpp"

#include <variant>

#include "can/core/device_info.hpp"
#include "can/core/dispatch.hpp"
#include "can/core/freertos_can_dispatch.hpp"
#include "can/core/message_writer.hpp"
#include "can/core/messages.hpp"
#include "can/firmware/hal_can_message_buffer.hpp"
#include "common/core/freertos_task.hpp"
#include "common/firmware/can.h"
#include "common/firmware/errors.h"
#include "common/firmware/i2c_comms.hpp"
#include "pipettes/core/eeprom.hpp"

using namespace hal_can_bus;
using namespace can_messages;
using namespace freertos_can_dispatch;
using namespace can_dispatch;
using namespace freertos_task;
using namespace can_message_writer;
using namespace i2c;
using namespace can_device_info;

extern FDCAN_HandleTypeDef fdcan1;

static auto can_bus_1 = HalCanBus(&fdcan1);
static auto message_writer_1 = MessageWriter(can_bus_1);

struct MotorHandler {
    using MessageType =
        std::variant<std::monostate, SetSpeedRequest, GetSpeedRequest,
                     StopRequest, GetStatusRequest, MoveRequest>;
    MotorHandler() {}
    MotorHandler(const MotorHandler &) = delete;
    MotorHandler(const MotorHandler &&) = delete;
    MotorHandler &operator=(const MotorHandler &) = delete;
    MotorHandler &&operator=(const MotorHandler &&) = delete;

    void handle(MessageType &m) {
        std::visit([this](auto o) { this->visit(o); }, m);
    }

    void visit(std::monostate &m) {}

    void visit(SetSpeedRequest &m) { message_writer_1.write(NodeId::host, m); }

    void visit(GetSpeedRequest &m) { message_writer_1.write(NodeId::host, m); }

    void visit(StopRequest &m) { message_writer_1.write(NodeId::gantry, m); }

    void visit(GetStatusRequest &m) { message_writer_1.write(NodeId::host, m); }

    void visit(MoveRequest &m) { message_writer_1.write(NodeId::host, m); }
};

struct EEPromHandler {
    using MessageType = std::variant<std::monostate, WriteToEEPromRequest,
                                     ReadFromEEPromRequest>;
    explicit EEPromHandler(I2C &i2c) : i2c(i2c) {}
    EEPromHandler(const EEPromHandler &) = delete;
    EEPromHandler(const EEPromHandler &&) = delete;
    EEPromHandler &operator=(const EEPromHandler &) = delete;
    EEPromHandler &&operator=(const EEPromHandler &&) = delete;
    I2C &i2c;

    void handle(MessageType &m) {
        std::visit([this](auto o) { this->visit(o); }, m);
    }

    void visit(std::monostate &m) {}

    void visit(WriteToEEPromRequest &m) { eeprom::write(i2c, m.serial_number); }

    void visit(ReadFromEEPromRequest &m) {
        const uint8_t serial_number = eeprom::read(i2c);
        auto message = ReadFromEEPromResponse{{}, serial_number};
        message_writer_1.write(NodeId::host, message);
    }
};

/** The parsed message handler */
static auto motor_handler = MotorHandler{};
static auto i2c_comms = I2C{};
static auto eeprom_handler = EEPromHandler{i2c_comms};
static auto device_info_handler =
    DeviceInfoHandler{message_writer_1, NodeId::pipette, 0};

/** The connection between the motor handler and message buffer */
static auto motor_dispatch_target = DispatchParseTarget<
    MotorHandler, can_messages::SetSpeedRequest, can_messages::GetSpeedRequest,
    can_messages::StopRequest, can_messages::GetStatusRequest,
    can_messages::MoveRequest>{motor_handler};

static auto eeprom_dispatch_target =
    DispatchParseTarget<EEPromHandler, can_messages::WriteToEEPromRequest,
                        can_messages::ReadFromEEPromRequest>{eeprom_handler};

static auto device_info_dispatch_target =
    DispatchParseTarget<decltype(device_info_handler),
                        can_messages::DeviceInfoRequest>{device_info_handler};

/** Dispatcher to the various handlers */
static auto dispatcher = Dispatcher(
    motor_dispatch_target, eeprom_dispatch_target, device_info_dispatch_target);

struct Task {
    [[noreturn]] void operator()() {
        if (MX_FDCAN1_Init(&fdcan1) != HAL_OK) {
            Error_Handler();
        }

        can_bus_1.start();

        auto poller = FreeRTOSCanBufferPoller(
            hal_can_message_buffer::get_message_buffer(), dispatcher);
        poller();
    }
};

static auto task_entry = Task{};

auto static task = FreeRTOSTask<256, 5, Task>("can task", task_entry);

void can_task::start() {}
