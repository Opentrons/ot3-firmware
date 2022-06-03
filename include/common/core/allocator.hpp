#include <algorithm>
#include <array>
#include <variant>

#include "common/core/logging.h"

/*
 * A generic pool allocator class that can be used to statically allocate memory
 * for typically dynamic objects (such as maps).
 *
 */

// should have type of object and max number of.
// should have backing be an array and/or pointer to array. Do not use malloc!
// only support allocating memory for one type of object at a time.

// Use concept to check that individual size
template <typename Element, std::size_t MaxElements>
class PoolAllocator {
  public:
    using value_type = Element;
    using size_type = std::size_t;
    using difference_type = ptrdiff_t;
    using pointer = Element*;
    using const_pointer = const Element*;
    using reference = Element&;
    using const_reference = const Element&;

    PoolAllocator() {}
    auto operator=(PoolAllocator&) -> PoolAllocator& = delete;
    auto operator=(PoolAllocator&&) -> PoolAllocator&& = delete;
    PoolAllocator(const PoolAllocator&) = default;
    PoolAllocator(PoolAllocator&&) = delete;
    ~PoolAllocator() = default;

    void deallocate(pointer loc, size_t num_blocks) {
        // Find index into backing array using the fact that loc is the first
        // item in the AllocatorElement struct and so has the same address.
        if (loc < backing.begin()) return;
        if ((loc - backing.begin()) % sizeof(*loc) != 0) return;
//        auto index = (reinterpret_cast<AllocatorElement<Element>*>(loc) -
//                      backing.begin()) /
//                     sizeof(AllocatorElement<Element>);
//        LOG("Deallocating %d", index);
//
//        if (index < MaxElements) {
//            backing[index].used = false;
//        }
//        auto starting_loc = &backing_flag[loc - backing.begin()];
//        std::fill(starting_loc, starting_loc + num_blocks, false);
        auto index = (loc - backing.begin()) / num_blocks;
        backing_flag[index] = false;
        LOG("Calling contiguous block deallocate and index starting %d", index);
        largest_contiguous_block();
    }

    pointer allocate(size_t num_blocks, const void* = 0) {
        if (max_size() < num_blocks) {
            // not sure if we need this
            throw std::bad_alloc();
        }

        if (current_available) {
            std::fill(current_available, current_available + num_blocks, true);
            auto pointer_alloc = &backing[current_available - backing_flag.begin()];
//            current_available += num_blocks;
            LOG("Calling contiguous allocate");
            largest_contiguous_block();
            return pointer_alloc;
        }
        throw std::bad_alloc();
    }

    void largest_contiguous_block() {
        // find next true from current available
        // if current available goes to end, do a reverse search.
        // TODO Can we make this slightly better than O(n)
        auto running_max_length = 0;
        auto max_length = 0;
        for (flag_pointer pt_loc = backing_flag.begin(); pt_loc < backing_flag.end(); pt_loc++) {
            if (*pt_loc == false) {
                if (max_length == 0 && pt_loc != current_available) {
                    current_available = pt_loc;
                }
                max_length++;
            } else if (*pt_loc == true) {
                max_length = 0;
            }
            running_max_length = std::max(max_length, running_max_length);
        }

        max_available_blocks = running_max_length;
        if (max_available_blocks == 0) {
            current_available = nullptr;
        }

    }

    size_type max_size() {
        return max_available_blocks;
    }

    template <class U>
    struct rebind {
        typedef PoolAllocator<U, MaxElements> other;
    };

  private:
    using MemoryType = std::array<Element, MaxElements>;
    using FlagMemoryType = std::array<bool, MaxElements>;
    using flag_pointer = bool*;
    MemoryType backing{};
    FlagMemoryType backing_flag{};
    std::size_t max_available_blocks = MaxElements;
    flag_pointer current_available = backing_flag.begin();
};
