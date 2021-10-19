#pragma once

#include <array>
#include <type_traits>
#include <variant>
#include <numeric>

#include "can/core/messages.hpp"

namespace move_group {


template <std::size_t MaxElements>
class MoveGroup {
  public:
    using MoveTypes =
        std::variant<std::monostate, can_messages::AddLinearMoveRequest>;

    MoveGroup() {}

    /**
     *
     * @param move
     */
    auto set_move(can_messages::AddLinearMoveRequest& move) -> bool {
        if (move.seq_id >= storage.size()) {
            // Error
            return false;
        }
        storage[move.seq_id] = move;
        return true;
    }

    /**
     *
     * @return
     */
    auto size() const -> std::size_t {
        int count = 0;
        for (const MoveTypes& m : storage) {
            if (!std::holds_alternative<std::monostate>(m)) {
                ++count;
            };
        }
        return count;
    }

    /**
     *
     * @return
     */
    auto empty() const -> bool {
        for (const MoveTypes& m : storage) {
            if (!std::holds_alternative<std::monostate>(m)) return false;
        }
        return true;
    }

    /**
     *
     */
    void clear() {
        for (MoveTypes& m : storage) {
            m = std::monostate{};
        }
    }

    /**
     *
     * @param seq_id
     * @return
     */
    const MoveTypes& get_move(uint8_t seq_id) const { return storage[seq_id]; }

    auto get_duration() const -> uint32_t {
        auto dash_fold = [](uint32_t accum, const MoveTypes& b) -> uint32_t {
            auto vv = std::visit(
                [](auto x) { return visit_duration(x); }, b);
            return accum + vv;
        };
        return std::accumulate(storage.cbegin(), storage.cend(), 0, dash_fold);
    }

  private:
    std::array<MoveTypes, MaxElements> storage{};

    static auto visit_duration(const std::monostate & m) -> uint32_t {
        return 0;
    }

    static auto visit_duration(const can_messages::AddLinearMoveRequest & m) -> uint32_t {
        return m.duration;
    }

};

template <std::size_t MaxMovesPerGroup, std::size_t MaxGroups>
using MoveGroupManager = std::array<MoveGroup<MaxMovesPerGroup>, MaxGroups>;

}  // namespace move_group
