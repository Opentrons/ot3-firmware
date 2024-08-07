#pragma once
namespace sensors {
namespace pie4ioe4 {
// https://www.diodes.com/assets/Datasheets/PI4IOE5V6408.pdf

constexpr uint16_t ADDRESS = 0x43 << 1;

enum class registers : uint8_t {
    dev_id_and_ctl = 0x01,  // default 1010 0010
    // reserved 0x02
    io_direction = 0x03,  // default 0000 0000
    // reserved 0x04
    output_state = 0x05,  // default 0000 0000
    // reserved 0x06
    output_high_impedance = 0x07,  // default 1111 1111
    // reserved 0x08
    input_default_state = 0x09,  // default 0000 0000
    // reserved 0x0A
    pull_up_down_enable = 0x0B,  // default 1111 1111
    // reserved 0x0C
    pull_up_down_select = 0x0D,  // default 0000 0000
    // reserved 0x0E
    input_status = 0x0F,  // default xxxx xxxx
    // reserved 0x10
    interrupt_mask = 0x11,  // default 0000 0000
    // reserved 0x12
    interrupt_status = 0x13,  // default xxxx xxxx
};

enum class version_responses : uint8_t {
    // right now only version D1.1 of the pipette sensor board responds
    // version 0 does not respond
    VERSION_1 = 0x01,  // Pipette_sensor board D1.1
};

}  // namespace pie4ioe4
}  // namespace sensors
