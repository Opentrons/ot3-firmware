#pragma once

#include <cstdint>

#include "can/core/ids.hpp"

namespace can::arbitration_id {

using namespace can::ids;

/**
 * A class that represents our 29-bit arbitration id.
 */
class ArbitrationId {
  public:
    ArbitrationId() = default;
    ArbitrationId(uint32_t arbitration_id) : id{arbitration_id} {}

    /**
     * Implicit conversion to uint32_t
     * @return the id
     */
    operator uint32_t() const { return get_id(); }

    /**
     * Get the encoded arbitration id as an integer
     * @return the id
     */
    [[nodiscard]] auto get_id() const -> uint32_t { return id; }

    [[nodiscard]] auto function_code() const -> FunctionCode {
        return static_cast<FunctionCode>(
            get(function_code_bit_mask, function_code_shift));
    }
    [[nodiscard]] auto node_id() const -> NodeId {
        return static_cast<NodeId>(get(node_id_bit_mask, node_id_shift));
    }
    [[nodiscard]] auto originating_node_id() const -> NodeId {
        return static_cast<NodeId>(
            get(originating_node_id_bit_mask, originating_node_id_shift));
    }
    [[nodiscard]] auto message_id() const -> MessageId {
        return static_cast<MessageId>(
            get(message_id_bit_mask, message_id_shift));
    }

    void function_code(FunctionCode v) {
        set(static_cast<uint32_t>(v), function_code_mask, function_code_shift);
    }
    void node_id(NodeId v) {
        set(static_cast<uint32_t>(v), node_id_mask, node_id_shift);
    }
    void originating_node_id(NodeId v) {
        set(static_cast<uint32_t>(v), node_id_mask, originating_node_id_shift);
    }
    void message_id(MessageId v) {
        set(static_cast<uint32_t>(v), message_id_mask, message_id_shift);
    }

    // Bits 0-3
    static constexpr auto function_code_mask = 0xF;
    static constexpr auto function_code_shift = 0;
    // Bits 4-10
    static constexpr auto node_id_mask = 0x7F;
    static constexpr auto node_id_shift = 4;
    // Bits 11-17
    static constexpr auto originating_node_id_shift = 11;
    // Bits 18-28
    static constexpr auto message_id_mask = 0x7FF;
    static constexpr auto message_id_shift = 18;

    static constexpr auto function_code_bit_mask = function_code_mask
                                                   << function_code_shift;
    static constexpr auto node_id_bit_mask = node_id_mask << node_id_shift;
    static constexpr auto originating_node_id_bit_mask =
        node_id_mask << originating_node_id_shift;
    static constexpr auto message_id_bit_mask = message_id_mask
                                                << message_id_shift;

  private:
    /**
     * Set bits in the id.
     * @param val The value.
     * @param mask The mask to apply.
     * @param shift The amount to left shift masked value
     */
    void set(uint32_t val, uint32_t mask, uint32_t shift) {
        // Clear bits
        id &= ~(mask << shift);
        // Write bits
        id |= ((val & mask) << shift);
    }

    /**
     * Get bits from the id.
     * @param mask the mask to apply.
     * @param shift amount to shift right.
     * @return the value
     */
    [[nodiscard]] auto get(uint32_t mask, uint32_t shift) const -> uint32_t {
        return (id & mask) >> shift;
    }

    uint32_t id{0};
};

}  // namespace can::arbitration_id
