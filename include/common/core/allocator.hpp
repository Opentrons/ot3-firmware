#include <algorithm>
#include <array>
#include <variant>

#include "common/core/logging.h"

/*
 * A generic pool allocator class that can be used to statically allocate memory
 * for typically dynamic objects (such as maps).
 *
 * Example Use-Case:
 *
 *
 */

template <typename Element>
struct AllocatorElement {
    // The address of element member must be the same as the address of an
    // AllocatorElement.
    // Thus, element must be first member of this struct. And there can be no
    // member functions and no private variables.
    // See https://en.cppreference.com/w/cpp/types/is_standard_layout
    Element element{};
    bool used = false;
};

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

    void deallocate(pointer loc, size_t) {
        // Find index into backing array using the fact that loc is the first
        // item in the AllocatorElement struct and so has the same address.
        auto index = (reinterpret_cast<AllocatorElement<Element>*>(loc) -
                      backing.begin()) /
                     sizeof(AllocatorElement<Element>);
        LOG("Deallocating %d", index);

        if (index < MaxElements) {
            backing[index].used = false;
        }
    }

    pointer allocate(size_t, const void* = 0) {
        auto iter = std::find_if_not(backing.begin(), backing.end(),
                                     [](auto& i) { return i.used; });
        LOG("Returned %d", iter != std::end(backing));
        if (iter != std::end(backing)) {
            iter->used = true;
            return &iter->element;
        }
        throw std::bad_alloc();
    }

    template <class U>
    struct rebind {
        typedef PoolAllocator<U, MaxElements> other;
    };

  private:
    // Use a struct allocator element and then can return the exact address
    // to the element!
    using MemoryType = std::array<AllocatorElement<Element>, MaxElements>;
    MemoryType backing{};
};
