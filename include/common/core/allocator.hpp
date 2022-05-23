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

    void free_block(void *ptr) {
//        free(ptr);
    }

//    void build() {
//        // Create a linked-list with all free positions
//        for (int i = 0; i < total_blocks; ++i) {
//            std::size_t address = (std::size_t) start_ptr + i * block;
//            free_memory_list.push((Node *) address);
//        }
//    }

    void deallocate(pointer loc, size_t elm_size) {
//        mem_used -= block;
//        free_memory_list.push((Node *) ptr);
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


//template<typename Alloc = std::allocator<void>>
//class CustomAllocator {
//  public:
//    using MemorySize;
//    auto operator=(CustomAllocator&) -> CustomAllocator& = delete;
//    auto operator=(CustomAllocator&&) -> CustomAllocator&& = delete;
//    Allocator(CustomAllocator&) = delete;
//    Allocator(CustomAllocator&&) = delete;
//
//    void deallocate(T * ptr, size_t size) {pAlloc->free_block(); }
//    pointer allocate(size_t size, const void * = 0) {
//        return pAlloc->next_block();
//    }
//  private:
//    Allocator<T> *pAlloc;
//};