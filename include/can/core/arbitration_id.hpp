#pragma once

#include <cstdint>

namespace can_arbitration_id {

/**
 * A class that represents our 29-bit arbitration id.
 */
class ArbitrationId {
  public:
    ArbitrationId() {}
    ArbitrationId(uint32_t arbitration_id) : id{arbitration_id} {}

    operator uint32_t() const { return get_id(); }
    [[nodiscard]] auto get_id() const -> uint32_t { return id; }

    [[nodiscard]] auto function_code() const -> uint8_t {
        return get(function_code_mask, function_code_shift);
    }
    [[nodiscard]] auto node_id() const -> uint8_t {
        return get(node_id_mask, node_id_shift);
    }
    [[nodiscard]] auto message_id() const -> uint16_t {
        return get(message_id_mask, message_id_shift);
    }

    void function_code(uint8_t v) {
        set(v, function_code_mask, function_code_shift);
    }
    void node_id(uint8_t v) { set(v, node_id_mask, node_id_shift); }
    void message_id(uint16_t v) { set(v, message_id_mask, message_id_shift); }

    // Bits 0-6
    static constexpr auto function_code_mask = 0x7F;
    static constexpr auto function_code_shift = 0;
    // Bits 7-14
    static constexpr auto node_id_mask = 0xFF;
    static constexpr auto node_id_shift = 7;
    // Bits 15-29
    static constexpr auto message_id_mask = 0x3FFF;
    static constexpr auto message_id_shift = 15;

    static constexpr auto function_code_bit_mask = function_code_mask
                                                   << function_code_shift;
    static constexpr auto node_id_bit_mask = node_id_mask << node_id_shift;
    static constexpr auto message_id_bit_mask = message_id_mask
                                                << message_id_shift;

  private:
    void set(uint32_t val, int mask, int shift) {
        id |= ((val & mask) << shift);
    }

    [[nodiscard]] auto get(int mask, int shift) const -> uint32_t {
        return (id >> shift) & mask;
    }

    uint32_t id{0};
};

}  // namespace can_arbitration_id