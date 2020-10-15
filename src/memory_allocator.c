#include "memory_allocator.h"
#include <stdlib.h>
#include <stdbool.h>
#define PLATFORM_ALIGNMENT 8

static const bool mask = 1;



//Auxiliary functions
void update_size_of_block(void* block_ptr,size_t size){
    ((size_t*)block_ptr)[0] = size;
}

bool is_free(void* block_ptr){
    return !(((size_t*)block_ptr)[0] & mask);
}

void update_indication_to_allocated(void* block_ptr){
    ((size_t*)block_ptr)[0] |= mask;
}

void update_indication_to_free(void* block_ptr){
    ((size_t*)block_ptr)[0] &= 0;
}

size_t get_block_size(void* block_ptr){
    return (((size_t*)block_ptr)[0]/2)*2;
}

void* get_ptr_to_payload(void* block_ptr){
    return ((size_t*)block_ptr)+1;
}

size_t get_size_aligned_to_platform(size_t size){
    return size += (size%PLATFORM_ALIGNMENT != 0)*PLATFORM_ALIGNMENT - size%PLATFORM_ALIGNMENT;
}

void split_block(void* block_ptr,size_t size){
    update_size_of_block(((char*)block_ptr+PLATFORM_ALIGNMENT+size),get_block_size(block_ptr) - size);
    update_size_of_block(block_ptr,size);
    update_indication_to_allocated(block_ptr);
}

void* get_next_block_ptr(void* block_ptr){
    return (char*)block_ptr+PLATFORM_ALIGNMENT+get_block_size(block_ptr);
}

void* get_prev_block_ptr(void* block_ptr){
    return ((size_t*)block_ptr)-1;
}

void free_bytes(void* ptr,size_t amount){
    size_t i;
    char* char_ptr = (char*)ptr;
    for(i=0;i<amount;++i){
        char_ptr[i] = 0;
    }
}

void* merge_two_adjacent_blocks(void* first_block_ptr){
    size_t* next_block_ptr = (size_t*)(get_next_block_ptr(first_block_ptr));
    size_t size_to_add = get_block_size(next_block_ptr);
    next_block_ptr[0] = 0;
    update_size_of_block(first_block_ptr, get_block_size(first_block_ptr)+size_to_add);
    return first_block_ptr;
}


//functions of API

MemoryAllocator* MemoryAllocator_init(void* memoryPool, size_t size){
    if(!memoryPool){
        return NULL;
    }

    MemoryAllocator* memoryAllocator = malloc(sizeof(MemoryAllocator));
    if(!memoryAllocator){
        return NULL;
    }

    memoryAllocator->memory_pool_ptr = memoryPool;
    update_size_of_block(memoryPool,size);
    return memoryAllocator;
}

void* MemoryAllocator_release(MemoryAllocator* allocator){
    if(!allocator){
        return NULL;
    }

    void* ptr = allocator->memory_pool_ptr;
    free(allocator);
    return ptr;
}

void* MemoryAllocator_allocate(MemoryAllocator* allocator, size_t size){
    if(!allocator){
        return NULL;
    }

    void* temp_memory_pool = allocator->memory_pool_ptr;
    size_t current_size;
    size_t i=0;
    size = get_size_aligned_to_platform(size);

    while(i < allocator->size){
        current_size = get_block_size(temp_memory_pool);

        if(is_free(temp_memory_pool)){
            printf("find free block\n");

            if(current_size==size){
                printf("suit block\n");
                update_indication_to_allocated(temp_memory_pool);
                return get_ptr_to_payload(temp_memory_pool);
            }

            if(size < current_size){
                printf("first allocate\n");
                split_block(temp_memory_pool,size);
                return get_ptr_to_payload(temp_memory_pool);
            }
            if(size > current_size){
                if(is_free(get_next_block_ptr(temp_memory_pool))) {
                    merge_two_adjacent_blocks(temp_memory_pool);
                    continue;
                }
            }
        }

        i+=current_size;
        temp_memory_pool = get_next_block_ptr(temp_memory_pool);
    }
    return NULL;
}

void MemoryAllocator_free(MemoryAllocator* allocator, void* ptr){
    if(!allocator || !ptr){
        return;
    }
    if(is_free(get_next_block_ptr(ptr))){
        free_bytes(((size_t*)ptr)+1,get_block_size(ptr)+8);
        update_size_of_block(ptr, get_block_size(ptr)+get_block_size(get_next_block_ptr(ptr)));
    }
    else{
        free_bytes(((size_t*)ptr)+1,get_block_size(ptr));
    }
    update_indication_to_free(ptr);
}

size_t MemoryAllocator_optimize(MemoryAllocator* allocator){
    if(!allocator){
        return 0;
    }
    size_t* temp_ptr = allocator->memory_pool_ptr;
    size_t max_size = 0, i=0, current_size = get_block_size(temp_ptr);

    while(i<allocator->size){
        if(current_size > max_size){
            max_size = current_size;
        }

        if(is_free(get_next_block_ptr(temp_ptr))){
            merge_two_adjacent_blocks(temp_ptr);
        }

        else{
            i += PLATFORM_ALIGNMENT + current_size;
            temp_ptr += 1 + current_size/PLATFORM_ALIGNMENT;
        }
    }
    return max_size;
}