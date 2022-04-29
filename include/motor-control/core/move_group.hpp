#pragma once

#include <array>
#include <numeric>
#include <type_traits>
#include <variant>

#include "can/core/messages.hpp"

namespace move_group {

template <typename Candidate>
concept Groupable = requires(Candidate C) {
    std::is_integral_v<decltype(C.duration)>;
    std::is_integral_v<decltype(C.group_id)>;
    std::is_integral_v<decltype(C.seq_id)>;
};

template <std::size_t GroupSize, Groupable... MoveStructs>
class MoveGroup {
  public:
    using MoveTypes = std::variant<std::monostate, MoveStructs...>;

    /**
     * Set a slot in the move group to a liner move.
     * @param move The CAN message.
     * @return True on success.
     */
    [[nodiscard]] auto set_move(auto&& move) -> bool {
        if (move.seq_id >= storage.size()) {
            // out of range error.
            return false;
        }
        storage.at(move.seq_id) = move;
        return true;
    }

    /**
     * Get the number of used slots in the move group.
     * @return int
     */
    [[nodiscard]] auto size() const -> std::size_t {
        std::size_t count = 0;
        for (const MoveTypes& m : storage) {
            if (!std::holds_alternative<std::monostate>(m)) {
                ++count;
            };
        }
        return count;
    }

    /**
     * Check if there any moves in the move group.
     * @return True if empty
     */
    [[nodiscard]] auto empty() const -> bool {
        for (const MoveTypes& m : storage) {
            if (!std::holds_alternative<std::monostate>(m)) {
                return false;
            }
        }
        return true;
    }

    /**
     * Clear all the moves in the group by marking them as std::monostate.
     */
    void clear() {
        for (MoveTypes& m : storage) {
            m = std::monostate{};
        }
    }

    /**
     * Get the move at sequence id
     *
     * @param seq_id index
     * @return The move type
     */
    [[nodiscard]] auto get_move(uint8_t seq_id) const -> const MoveTypes& {
        // TODO (al, 2021-10-20): Check bounds
        return storage.at(seq_id);
    }

    /**
     * Return the total duration of all the moves in the move group.
     * @return Duration
     */
    [[nodiscard]] auto get_duration() const -> uint32_t {
        auto accumulate_duration = [](uint32_t accum,
                                      const MoveTypes& b) -> uint32_t {
            auto vv = std::visit([](auto x) { return visit_duration(x); }, b);
            return accum + vv;
        };
        return std::accumulate(storage.cbegin(), storage.cend(), 0,
                               accumulate_duration);
    }

  private:
    std::array<MoveTypes, GroupSize> storage{};

    static auto visit_duration(const std::monostate&) -> uint32_t { return 0; }

    static auto visit_duration(const auto& m) -> uint32_t { return m.duration; }
};

template <std::size_t GroupCount, std::size_t GroupSize,
          Groupable... MoveStructs>
using MoveGroupManager =
    std::array<MoveGroup<GroupSize, MoveStructs...>, GroupCount>;

}  // namespace move_group
