#pragma once

#include <array>
#include <cstdint>
#include <span>

#include "common/core/bit_utils.hpp"
#include "common/core/message_queue.hpp"
#include "motor-control/core/motor_messages.hpp"
#include "motor-control/core/spi.hpp"

/*
 * Test mock for Motor
 */

using namespace bit_utils;
using namespace motor_messages;

namespace test_motor_control {

const uint8_t WRITE = 0x80;

enum class MotorRegisters : uint8_t {
    GCONF = 0x00,
    DRVSTATUS = 0x6F,
};

static constexpr auto BUFFER_SIZE = 5;
using BufferType = std::array<uint8_t, BUFFER_SIZE>;

template <spi::TMC2130Spi SpiDriver>
class TestMotorDriver {
  public:
    TestMotorDriver(SpiDriver& spi) : spi_comms(spi) {}
    std::string state = "init";
    void setup() { state = "setup"; }
    void get_status() {
        status = 0x0;
        data = 0x0;
        state = "got_status";
    }
    [[nodiscard]] auto get_current_status() const -> uint8_t { return status; }
    [[nodiscard]] auto get_current_data() const -> uint32_t { return data; }
    static void process_buffer(const BufferType& rxBuffer, uint8_t& status,
                               uint32_t& data) {
        auto iter = rxBuffer.cbegin();
        iter = bit_utils::bytes_to_int(iter, rxBuffer.cend(), status);
        // NOLINTNEXTLINE(clang-diagnostic-unused-result)
        iter = bit_utils::bytes_to_int(iter, rxBuffer.cend(), data);
    }

  private:
    uint8_t status = 0x0;
    uint32_t data = 0x0;
    SpiDriver& spi_comms;
    void build_command(uint8_t command, uint32_t& command_data,
                       std::array<uint8_t, 5>& txBuffer) {}

    void reset_data() { data = 0x0; }
    void reset_status() { status = 0; }
};

template <spi::TMC2130Spi SpiDriver, template <class> class QueueImpl>
requires MessageQueue<QueueImpl<Move>, Move>
class TestMotionController {
  public:
    using GenericQueue = QueueImpl<Move>;
    TestMotionController(SpiDriver& spi, GenericQueue& queue)
        : spi_comms(spi), queue(queue) {}
    void set_speed(uint32_t s) { speed = s; }
    void set_direction(bool d) { direction = d; }
    void set_acceleration(uint32_t a) { acc = a; }
    void set_distance(uint32_t d) { dist = d; }
    void move(const CanMove& msg) { speed = 10000; }
    void stop() { speed = 0; }
    uint32_t get_acceleration() { return acc; }
    uint32_t get_speed() { return speed; }
    bool get_direction() { return direction; }

  private:
    uint32_t acc = 0x0;
    uint32_t speed = 0x0;
    uint32_t dist = 0x0;
    bool direction = true;
    SpiDriver& spi_comms;
    GenericQueue& queue;
};

template <spi::TMC2130Spi SpiDriver, template <class> class QueueImpl>
requires MessageQueue<QueueImpl<Move>, Move>
struct TestMotor {
    using GenericQueue = QueueImpl<Move>;
    explicit TestMotor(SpiDriver& spi, GenericQueue& queue)
        : spi_comms(spi), queue(queue) {}
    SpiDriver& spi_comms;
    GenericQueue& queue;
    TestMotorDriver<SpiDriver> driver = TestMotorDriver{spi_comms};
    TestMotionController<SpiDriver, QueueImpl> motion_controller =
        TestMotionController{spi_comms, queue};
};

}  // namespace test_motor_control
