#pragma once

#include <algorithm>
#include <array>
#include <optional>
#include <variant>

namespace i2c {
namespace transaction {

/**
 * A class that maps i2c transaction ids to arbitrary objects. This class is not
 * reentrant.
 *
 * @tparam Entry The type mapped to a transaction id
 * @tparam MAX_SIZE The maximum number of entries in the collection
 */
template <typename Entry, std::size_t MAX_SIZE>
class IdMap {
  public:
    /**
     * Add an entry to the collection
     * @param t Entry
     * @return returns an optional containing the id on success. No id if
     * collection is full.
     */
    auto add(const Entry& t) -> std::optional<uint32_t> {
        auto iter = std::find_if(
            transactions.begin(), transactions.end(),
            [](auto& i) { return std::holds_alternative<std::monostate>(i); });
        if (iter != std::end(transactions)) {
            *iter = t;
            return iter - transactions.begin();
        }
        return std::nullopt;
    }

    /**
     * Remove mapping from the collection.
     * @param id the id returned from add
     * @return The existing entry if it exists
     */
    auto remove(uint32_t id) -> std::optional<Entry> {
        if (static_cast<std::size_t>(id) < transactions.size()) {
            auto i = transactions[id];
            if (std::holds_alternative<Entry>(i)) {
                auto transaction = std::get<Entry>(i);
                transactions[id] = std::monostate{};
                return transaction;
            }
        }
        return std::nullopt;
    }

    /**
     * Get the number of mappings.
     * @return the count.
     */
    auto count() -> int {
        return std::count_if(
            transactions.begin(), transactions.end(),
            [](auto& i) { return !std::holds_alternative<std::monostate>(i); });
    }

  private:
    using Transaction = std::variant<std::monostate, Entry>;
    using Transactions = std::array<Transaction, MAX_SIZE>;
    Transactions transactions{};
};

}  // namespace transaction
}  // namespace i2c