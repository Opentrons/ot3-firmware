#include <array>

#include<variant>
#include<algorithm>

/*
 * A generic pool allocator class that can be used to statically allocate memory
 * for typically dynamic objects (such as maps).
 *
 * Example Use-Case:
 *
 *
 */
//
//template <typename TotalMemoryBlock, typename IndividualBlock>
//concept Allocator = requires(TotalMemoryBlock m, IndividualBlock b) {
//    std::is_integral<m>;
//    std::is_integral<b>;
//    {m / b} -> std::convertable_to<int>;
//};

template<typename Element>
struct AllocatorElement {
    Element element = nullptr;
    bool used = false;
};

// should have type of object and max number of.
// should have backing be an array and/or pointer to array. Do not use malloc!
//only support allocating memory for one type of object at a time.

// Use concept to check that individual size
template<typename Element, std::size_t MaxElements>
class PoolAllocator {
  public:
    using value_type = Element;
    using size_type = std::size_t;
    using difference_type = ptrdiff_t;
    using pointer = Element*;
    using const_pointer = const Element*;
    using reference = Element&;
    using const_reference = const Element&;

    PoolAllocator() {
    }
    auto operator=(PoolAllocator&) -> PoolAllocator& = delete;
    auto operator=(PoolAllocator&&) -> PoolAllocator&& = delete;
    PoolAllocator(PoolAllocator&) = delete;
    PoolAllocator(PoolAllocator&&) = delete;
    ~PoolAllocator() = default;

    void deallocate(pointer loc, size_t elm_size) {
       auto index = (loc - backing.begin()) / elm_size;
       backing_flag[index] = false;
    }

    pointer allocate(size_t, const void * = 0) {
        auto iter = std::find_if(backing_flag.begin(), backing_flag.end(), [](auto&i) {return !i;});
        if (iter) {
            *iter = true;
            return &backing[iter - backing_flag.begin()];
        }
        return nullptr;
    }

    template <class U>
    struct rebind { typedef PoolAllocator<U, MaxElements> other; };

  private:
    // Use a struct allocator element and then can return the exact address
    // to the element!
    using MemoryType = std::array<Element, MaxElements>;
    using FlagMemoryType = std::array<bool, MaxElements>;
    MemoryType backing{};
    FlagMemoryType backing_flag{};
};
