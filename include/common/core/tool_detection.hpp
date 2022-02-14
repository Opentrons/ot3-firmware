#pragma once
#include <cstdint>
#include <ranges>
#include <span>

#include "can/core/ids.hpp"

namespace tool_detection {

// A quick specialization that is syntactically meaningless but a check
// to the programmer
using millivolts_t = uint16_t;

// Contains the bounds for value checking
struct ToolCheckBounds {
    millivolts_t upper;
    millivolts_t lower;
};

// Which carrier a device is attached to
enum Carrier : uint8_t {
    GRIPPER_CARRIER = 0x00,
    Z_CARRIER = 0x01,
    A_CARRIER = 0x02,
    UNKNOWN = 0x03,
};

// A full specification of an attached tool, including its bounds
struct Tool {
    can_ids::ToolType tool_type;
    Carrier tool_carrier;
    ToolCheckBounds bounds;
    [[nodiscard]] auto within_bounds(millivolts_t reading) const -> bool {
        return ((reading >= this->bounds.lower) &&
                (reading < this->bounds.upper));
        return false;
    }
};

// Get a view on the lookup table for bounds -> tool values, including versions
// with filtering for cases where you only care about one carrier or tool type.
// This can be useful because if you only have a specific set of physically
// reasonable outcomes - for instance, it's not reasonable to see a tool
// physically attached to the A mount pins having Z mount values - the
// filtering prevents such strange values from happening
auto lookup_table() -> std::span<Tool>;


#if !defined(OPENTRONS_CLANG_TIDY_WORKAROUND_44178)
// Clang 12 and 13 and head currently have a bug in the way they handle specific
// uses of template concepts that stems from a disagreement about interpretation
// of standards language between them, and, well, everybody else. This bug
// completely breaks the compilation of half of std::ranges, which we use extensively
// here. The bug: https://github.com/llvm/llvm-project/issues/44178
// Since we only ever compile tests and firmware and simulation with gcc, it's not
// really a problem, but it does break linting, which uses clang-tidy and therefore
// clang. Our options are limited here; the only way I could see to actually get
// to use this code is to have dummy versions of the functions here that get used
// during lint.

// Limit filter options to one of the two available pieces of metadata
template <typename T>
concept ToolOrCarrier =
    std::is_same_v<T, Carrier> || std::is_same_v<T, can_ids::ToolType>;

// Looks like a linear container and contains a Tool
template <typename T>
concept ToolList = std::ranges::view<T> &&
    std::is_same_v<std::iter_value_t<std::ranges::iterator_t<T>>, Tool>;

// Lookup table retrieval with filtering. Does the same thing as lookup_table(),
// but with a return type that is annoying enough to deal with that we let it be
// fully auto deduced.
//
// Can specify your own base lookup table (including, possibly, the output of
// another filter function call) or load from the default one.
template <ToolOrCarrier FilterTarget>
auto lookup_table_filtered(FilterTarget filter_by) {
    return filter_lookup_table(filter_by, lookup_table());
}

// This is a template overload set, where the template argument decides the
// lookup table type (necessary to deal with the std::views stuff) and the
// overloading handles the two different filter types, which need to access
// different struct members
template <ToolList Lookup>
auto filter_lookup_table(can_ids::ToolType filter_by, Lookup from_table) {
    return std::views::filter(from_table, [filter_by](const auto& elem) {
        return elem.tool_type == filter_by;
    });
};

template <ToolList Lookup>
auto filter_lookup_table(Carrier filter_by, Lookup from_table) {
    return std::views::filter(from_table, [filter_by](const auto& elem) {
        return elem.tool_carrier == filter_by;
    });
}

// Get a full tool specification. If the lookup table isn't specified, the
// full builtin table is used.
template <ToolList Lookup>
auto tool_from_reading(millivolts_t reading, Lookup with_lookup) -> Tool {
    for (const auto& element : with_lookup) {
        if (element.within_bounds(reading)) {
            return element;
        }
    }
    return Tool{.tool_type = can_ids::ToolType::undefined_tool,
                .tool_carrier = Carrier::UNKNOWN,
                .bounds = {0, 0}};
}

inline auto tool_from_reading(millivolts_t reading) -> Tool {
    return tool_from_reading(reading, lookup_table());
}

// Turn a reading into specifically the carrier holding the tool. If the
// lookup table isn't specified, the full built in table is used.
template <ToolList Lookup>
auto carrier_from_reading(millivolts_t reading, Lookup with_lookup) -> Carrier {
    return tool_from_reading(reading, with_lookup).tool_carrier;
};
inline auto carrier_from_reading(millivolts_t reading) -> Carrier {
    return carrier_from_reading(reading, lookup_table());
}

// Turn a reading into specifically the tool type attached. If the
// lookup table isn't specified, the full built in table is used.
template <ToolList Lookup>
auto tooltype_from_reading(millivolts_t reading,
                           Lookup with_lookup) -> can_ids::ToolType {
    return tool_from_reading(reading, with_lookup).tool_type;
};

inline auto tooltype_from_reading(millivolts_t reading) -> can_ids::ToolType {
    return tooltype_from_reading(reading, lookup_table());
}
#else // defined(OPENTRONS_CLANG_TIDY_WORKAROUND_44178)
template <typename FilterTarget>
auto lookup_table_filtered(FilterTarget filter_by) {
    return filter_lookup_table(filter_by, lookup_table());
}

template <typename Lookup>
auto filter_lookup_table(can_ids::ToolType filter_by, Lookup from_table) {
    return from_table;
};

template <typename Lookup>
auto filter_lookup_table(Carrier filter_by, Lookup from_table) {
    return from_table;
}

template <typename Lookup>
auto tool_from_reading(millivolts_t reading, Lookup with_lookup) -> Tool {
    for (const auto& element : with_lookup) {
        if (element.within_bounds(reading)) {
            return element;
        }
    }
    return Tool{.tool_type = can_ids::ToolType::undefined_tool,
                .tool_carrier = Carrier::UNKNOWN,
                .bounds = {0, 0}};
}

inline auto tool_from_reading(millivolts_t reading) -> Tool {
    return tool_from_reading(reading, lookup_table());
}

template <typename Lookup>
auto carrier_from_reading(millivolts_t reading, Lookup with_lookup) -> Carrier {
    return tool_from_reading(reading, with_lookup).tool_carrier;
};
inline auto carrier_from_reading(millivolts_t reading) -> Carrier {
    return carrier_from_reading(reading, lookup_table());
}

template <typename Lookup>
auto tooltype_from_reading(millivolts_t reading,
                           Lookup with_lookup) -> can_ids::ToolType {
    return tool_from_reading(reading, with_lookup).tool_type;
};

inline auto tooltype_from_reading(millivolts_t reading) -> can_ids::ToolType {
    return tooltype_from_reading(reading, lookup_table());
}
#endif // defined(OPENTRONS_CLANG_TIDY_WORKAROUND_44178)


};  // namespace tool_detection
