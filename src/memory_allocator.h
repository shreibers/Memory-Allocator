

#include <stdio.h>

#ifndef MEMORYALLOCATOR_MEMORY_ALLOCATOR_H
#define MEMORYALLOCATOR_MEMORY_ALLOCATOR_H


typedef struct MemoryAllocator{
    void* memory_pool_ptr;
    size_t size;
}MemoryAllocator;

MemoryAllocator* MemoryAllocator_init(void* memoryPool, size_t size);

void*  MemoryAllocator_release(MemoryAllocator* allocator);

void*  MemoryAllocator_allocate(MemoryAllocator* allocator, size_t size);

void MemoryAllocator_free(MemoryAllocator* allocator, void* ptr);

size_t MemoryAllocator_optimize(MemoryAllocator* allocator);

#endif //MEMORYALLOCATOR_MEMORY_ALLOCATOR_H
