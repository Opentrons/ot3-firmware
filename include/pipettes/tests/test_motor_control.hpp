#pragma once

#include <array>
#include <cstdint>
#include <span>

#include "common/core/bit_utils.hpp"

/*
 * Test mock for Motor
 */

using namespace bit_utils;

namespace test_motor_control {

const uint8_t WRITE = 0x80;

enum class MotorRegisters : uint8_t {
    GCONF = 0x00,
    DRVSTATUS = 0x6F,
};

static constexpr auto BUFFER_SIZE = 5;
using BufferType = std::array<uint8_t, BUFFER_SIZE>;

class TestMotorDriver {
  public:
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
    void build_command(uint8_t command, uint32_t& command_data,
                       std::array<uint8_t, 5>& txBuffer) {}

    void reset_data() { data = 0x0; }
    void reset_status() { status = 0; }
};

class TestMotionController {
  public:
    void set_speed(uint32_t s) { speed = s; }
    void set_direction(bool d) { direction = d; }
    void set_acceleration(uint32_t a) { acc = a; }
    void set_distance();
    void move() { speed = 10000; }
    void stop() { speed = 0; }
    uint32_t get_acceleration() { return acc; }
    uint32_t get_speed() { return speed; }
    bool get_direction() { return direction; }

  private:
    uint32_t acc = 0x0;
    uint32_t speed = 0x0;
    bool direction = true;
};

class TestMotor {
  public:
    TestMotorDriver driver = TestMotorDriver{};
    TestMotionController motion_controller = TestMotionController{};
};

}  // namespace test_motor_control
