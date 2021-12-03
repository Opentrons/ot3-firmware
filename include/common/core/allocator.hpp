#include <concepts>
#include <memory>

#include "common/core/linkedlist.hpp"

/*
 * A generic pool allocator class that can be used to statically allocate memory
 * for typically dynamic objects (such as maps).
 *
 * Example Use-Case:
 *
 *
 */

template <typename TotalMemoryBlock, typename IndividualBlock>
concept Allocator = requires(TotalMemoryBlock m, IndividualBlock b) {
    std::is_integral<m>;
    std::is_integral<b>;
    {m / b} -> std::convertable_to<int>;
};

// should have type of object and max number of.
// should have backing be an array and/or pointer to array. Do not use malloc!
//only support allocating memory for one type of object at a time.

// Use concept to check that individual size
template<typename TotalMemoryBlock, typename IndividualBlock>
requires Allocator<TotalMemoryBlock, IndividualBlock>
class PoolAllocator {
  public:
    PoolAllocator() memory(TotalMemoryBlock), chunks(IndividualBlock) : {
        start_ptr = malloc(memory);
        total_blocks = memory / chunks;
        build();
    }
    auto operator=(PoolAllocator&) -> PoolAllocator& = delete;
    auto operator=(PoolAllocator&&) -> PoolAllocator&& = delete;
    PoolAllocator(PoolAllocator&) = delete;
    PoolAllocator(PoolAllocator&&) = delete;
    ~PoolAllocator() = free(start_ptr);

    void free_block(void *ptr) {
        free(ptr);
    }

    void build() {
        // Create a linked-list with all free positions
        for (int i = 0; i < total_blocks; ++i) {
            std::size_t address = (std::size_t) start_ptr + i * block;
            free_memory_list.push((Node *) address);
        }
    }

    void deallocate(void * ptr, size_t size) {
        mem_used -= block;
        free_memory_list.push((Node *) ptr);
    }

    void * allocate(size_t size, const void * = 0) {
        mem_used += block;
        return free_memory_list.pop();

    }

    void reset() {
        mem_used = 0;
    }

  private:
    int memory;
    int block;
    int total_blocks;
    int mem_used = 0;
    void * start_ptr = nullptr;

    struct  LinkedListStruct{};
    using Node = linkedlist::StackLinkedList<LinkedListStruct>::Node;
    linkedlist::StackLinkedList<LinkedListStruct> free_memory_list;
}


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