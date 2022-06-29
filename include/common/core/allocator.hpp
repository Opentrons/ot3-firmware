#include <algorithm>
#include <array>
#include <variant>

#include "common/core/logging.h"

/**
 * A generic custom allocator class that can be used to statically
 * allocate memory for typically dynamic objects (such as maps).
 *
 * To use this class, you must pass in the CustomAllocator class
 * to the template of any stl library you would like to use.
 *
 * For example, to use this class with map
 * you would do the following:
 *
 * std::map<
 *      std::string, int, std::less<std::string>,
 *      CustomAllocator<std::pair<const std::string, int>, max_entries>
 * > subject{};
 *
 * Caveats:
 * Any containers that reallocate every time (vectors for example) a
 * new element is added can only support fully allocated objects.
 * For example, say you have a vector:
 *
 * max entries = 2;
 * std::vector<int, CustomAllocator<int, max_entries>> subject{2, 3};
 *
 * You can change the elements of this array, but you cannot
 * dynamically re-size it.
 *
 * If you tried to insert another element like:
 *
 * subject.push_back(4)
 *
 * it will throw a bad alloc which is expected behavior. However, vectors will
 * also throw bad allocs if things haven't previously been allocated.
 * For example, say you have a vector defined with no elements:
 *
 * max entries = 2;
 * std::vector<int, CustomAllocator<int, max_entries>> subject;
 *
 * subject.push_back(1);
 * subject.push_back(2); // throws bad alloc
 *
 * This will throw a bad alloc because `subject.push_back(2)` tries
 * to allocate memory for both integers 1 and 2 which
 * is more space than available.
 **/

template <typename Element, std::size_t MaxElements>
class CustomAllocator {
  public:
    using value_type = Element;
    using size_type = std::size_t;
    using difference_type = ptrdiff_t;
    using pointer = Element*;
    using flag_pointer = bool*;
    using const_pointer = const Element*;
    using reference = Element&;
    using const_reference = const Element&;

    CustomAllocator() {}
    auto operator=(CustomAllocator&) -> CustomAllocator& = delete;
    auto operator=(CustomAllocator&&) -> CustomAllocator&& = delete;
    CustomAllocator(const CustomAllocator&) = default;
    CustomAllocator(CustomAllocator&&) = delete;
    ~CustomAllocator() = default;

    void deallocate(pointer loc, size_type num_blocks) {
        /*
         * Deallocate the number of blocks of memory, starting
         * from the location passed in.
         *
         * @param loc - The pointer in memory of the backing array
         * @param num_blocks - The number of contiguous blocks of memory
         */
        if (loc < backing.begin()) return;
        if ((loc - backing.begin()) % sizeof(*loc) != 0) return;

        auto index = (loc - backing.begin()) / num_blocks;
        auto backing_flag_ptr = &backing_flag[index];
        std::fill(backing_flag_ptr, backing_flag_ptr + num_blocks, false);
    }

    pointer allocate(size_type num_blocks, const void* = 0) {
        /*
         * Allocate the number of blocks of memory, starting
         * from the smallest available block.
         *
         * @param num_blocks - The number of blocks to allocate
         */
        auto allocate_block = next_contiguous_block(num_blocks);
        std::fill(allocate_block, allocate_block + num_blocks, true);
        return &backing[allocate_block - backing_flag.begin()];
    }

    flag_pointer next_contiguous_block(size_type num_blocks) {
        /*
         * A helper function to figure out where the current available
         * contiguous memory blocks are. We do this by linearly searching
         * through the array.
         */
        auto region_end = backing_flag.begin();
        for (flag_pointer region_begin = backing_flag.begin();
             region_begin <= backing_flag.end(); region_begin++) {
            if (*region_begin == true && region_begin == backing_flag.end()) {
                throw std::bad_alloc();
            }
            region_end = region_begin;
            while (*region_end == false && region_end != backing_flag.end() &&
                   static_cast<size_type>(region_end - region_begin) <
                       num_blocks) {
                region_end++;
            }
            if (static_cast<size_type>(region_end - region_begin) >=
                num_blocks) {
                return region_begin;
            }
            region_begin = region_end;
        }
        throw std::bad_alloc();
    }

    size_type max_size() {
        /*
         * The max size of this allocator. It does not currently tell you
         * the maximum available size.
         */
        return max_available_blocks;
    }

    template <class U>
    struct rebind {
        typedef CustomAllocator<U, MaxElements> other;
    };

    constexpr CustomAllocator select_on_container_copy_construction(
        const CustomAllocator& a) {
        return a;
    }

  private:
    using MemoryType = std::array<Element, MaxElements>;
    using FlagMemoryType = std::array<bool, MaxElements>;
    MemoryType backing{};
    FlagMemoryType backing_flag{};
    const std::size_t max_available_blocks = MaxElements;
};
