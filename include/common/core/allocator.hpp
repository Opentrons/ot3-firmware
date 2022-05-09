#include <array>

#include<variant>

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

    void deallocate(void *, size_t) {
//        mem_used -= block;
//        free_memory_list.push((Node *) ptr);
    }

    pointer allocate(size_t, const void * = 0) {
        return nullptr;

    }

    void reset() {
//        mem_used = 0;
    }

    template <class U>
    struct rebind { typedef PoolAllocator<U, MaxElements> other; };

  private:
//    int memory;
//    int block;
//    int total_blocks;
//    int mem_used = 0;
//    void * start_ptr = nullptr;

    using ElementType = std::variant<std::monostate, Element>;
    using MemoryType = std::array<ElementType, MaxElements>;
    MemoryType backing{};
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