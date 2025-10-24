#include <stddef.h>

#include <stdlib.h> //just for the EXIT_FAILURE
#include <string.h> //just to use strcpy
#include <stdio.h> //just for checking some shit

#define HEAP_SIZE 16 * 1024

static unsigned char heap[HEAP_SIZE];

#ifdef TRIVIAL_ALLOCATOR

// =============================================
// VERSION 1: TRIVIAL ALLOCATOR (ALLOCATION ONLY)
// =============================================

static size_t current_offset = 0;

void *memalloc(int size) {
    if(size <= 0) {
        return NULL;
    }

    if(current_offset + size > HEAP_SIZE) {
        fprintf(stderr, "Error: Not enough memory available! Requested: %d, Available: %zu\n", size, HEAP_SIZE - current_offset);
        exit(EXIT_FAILURE);
    }

    void *mem = &heap[current_offset];
    current_offset += size;
    return mem;
}

void memfree(void *ptr) {
    // dont do shit because the trivial versions doesn't support deallocation
    (void)ptr;
}

#else

// =============================================
// VERSION 2: FREE LIST ALLOCATOR (COMPLETE)
// =============================================

typedef struct memory_zone {
    size_t size;
    struct memory_zone *next;
    int is_free;
} memory_zone_t;

static memory_zone_t *free_list = NULL;

void meminit(void) {
    free_list = (memory_zone_t *)heap;
    free_list->size = HEAP_SIZE - sizeof(memory_zone_t);
    free_list->next = NULL;
    free_list->is_free = 1;

    printf("Heap initialized. Total size: %d bytes\n", HEAP_SIZE);
    printf("First free block size: %zu bytes\n", free_list->size);
}

void *memalloc(size_t size) {
    if(size <= 0) {
        return NULL;
    }

    memory_zone_t *prev = NULL;
    memory_zone_t *curr = free_list;

    // we're going to use the first-fit strat, so we search the frist block that is large 
    // enough to support all the memory that we want to allocate
    while(curr != NULL) {
        // found suitable block
        if(curr->is_free && curr->size >= size) {
            // split the block if its too big
            if(curr->size >= size + sizeof(memory_zone_t) + 1) {
                memory_zone_t *new_zone = (memory_zone_t *)((unsigned char *)curr 
                    + sizeof(memory_zone_t) + size);
                new_zone->size = curr->size - size - sizeof(memory_zone_t);
                new_zone->next = curr->next;
                new_zone->is_free = 1;

                curr->size = size;
                curr->next = new_zone;
                curr->is_free = 0;

                if(prev) prev->next = new_zone;
                else free_list = new_zone;
            } else {
                // here we use all the block
                if(prev) prev->next = curr->next;
                else free_list = curr->next;
            }

            curr->is_free = 0;
            curr->next = NULL;

            return (unsigned char*)curr + sizeof(memory_zone_t);
        }
        prev = curr;
        curr = curr->next;
    }

    fprintf(stderr, "Error: No suitable memory block found for size %ld\n", size);
    return NULL;
}

void memfree(void *ptr) {
    if(ptr == NULL) {
        return;
    }

    memory_zone_t *block_to_free = (memory_zone_t *)((unsigned char *)ptr 
        - sizeof(memory_zone_t));

    if((unsigned char*)block_to_free < heap || 
        (unsigned char *)block_to_free >= heap + HEAP_SIZE) {
        fprintf(stderr, "Warning: Attempt to free invalid pointer %p\n", ptr);
        return; 
    }

    if(block_to_free->is_free) {
        fprintf(stderr, "Warning: Double free detected for pointer %p\n", ptr);
        return;
    }

    block_to_free->is_free = 1;

    block_to_free->next = free_list;
    free_list = block_to_free;

    memory_zone_t *curr = free_list;
    while(curr != NULL && curr->next != NULL) {
        memory_zone_t *next_block = curr->next;

        if((unsigned char *)curr + sizeof(memory_zone_t) 
            + curr->size == (unsigned char *)next_block) {
            curr->size += sizeof(memory_zone_t) + next_block->size;
            curr->next = next_block->next;
        } else {
            curr = curr->next;
        }
    }
}

#endif

// =============================================
// TEST FUNCTIONS
// =============================================

void print_memory_status(void) {
#ifndef TRIVIAL_ALLOCATOR
    printf("\n=== Memory Status ===\n");
    
    memory_zone_t *current = free_list;
    size_t total_free = 0;
    int free_blocks = 0;
    
    printf("Free blocks:\n");
    while (current != NULL) {
        if (current->is_free) {
            printf("  Block at %p: size = %zu bytes\n", 
                   (void *)current, current->size);
            total_free += current->size;
            free_blocks++;
        }
        current = current->next;
    }
    
    printf("Total free blocks: %d, Total free memory: %zu bytes\n", 
           free_blocks, total_free);
    printf("=====================\n");
#endif
}

void test_trivial_allocator(void) {
#ifdef TRIVIAL_ALLOCATOR
    printf("=== Testing Trivial Allocator ===\n");
    
    int *array1 = (int *)memalloc(10 * sizeof(int));
    char *str = (char *)memalloc(100);
    double *array2 = (double *)memalloc(5 * sizeof(double));
    
    printf("Allocated addresses:\n");
    printf("array1 (40 bytes): %p\n", (void *)array1);
    printf("str    (100 bytes): %p\n", (void *)str);
    printf("array2 (40 bytes): %p\n", (void *)array2);
    
    // Test using the memory
    for (int i = 0; i < 10; i++) {
        array1[i] = i * i;
    }
    
    strcpy(str, "Hello, Memory Allocator!");
    printf("String content: %s\n", str);
    
    memfree(array1);  // Does nothing in trivial version
    printf("=== Trivial Allocator Test Complete ===\n\n");
#endif
}

void test_free_list_allocator(void) {
#ifndef TRIVIAL_ALLOCATOR
    printf("=== Testing Free List Allocator ===\n");
    
    meminit();
    print_memory_status();
    
    // Test allocation
    int *array1 = (int *)memalloc(10 * sizeof(int));
    char *str1 = (char *)memalloc(100);
    double *array2 = (double *)memalloc(5 * sizeof(double));
    
    printf("First allocation:\n");
    printf("array1: %p\n", (void *)array1);
    printf("str1:   %p\n", (void *)str1);
    printf("array2: %p\n", (void *)array2);
    print_memory_status();
    
    // Test using memory
    for (int i = 0; i < 10; i++) {
        array1[i] = i;
    }
    strcpy(str1, "First allocation");
    
    // Test freeing and reallocation
    memfree(str1);
    printf("After freeing str1:\n");
    print_memory_status();
    
    char *str2 = (char *)memalloc(50);
    printf("Reallocated str2: %p\n", (void *)str2);
    print_memory_status();
    
    // Test fragmentation and coalescing
    memfree(array1);
    printf("After freeing array1:\n");
    print_memory_status();
    
    memfree(array2);
    printf("After freeing array2 (should coalesce):\n");
    print_memory_status();
    
    // Test large allocation after coalescing
    void *large_block = memalloc(2000);
    printf("Large allocation (2000 bytes): %p\n", (void *)large_block);
    print_memory_status();
    
    printf("=== Free List Allocator Test Complete ===\n\n");
#endif
}

void test_edge_cases(void) {
    printf("=== Testing Edge Cases ===\n");
    
    // Test zero size
    void *zero_alloc = memalloc(0);
    printf("Allocation of size 0: %p\n", zero_alloc);
    
    // Test free of NULL
    memfree(NULL);
    printf("Free of NULL completed safely\n");
    
#ifndef TRIVIAL_ALLOCATOR
    // Test double free (only relevant for free list version)
    int *test_ptr = (int *)memalloc(sizeof(int));
    memfree(test_ptr);
    memfree(test_ptr);  // Should print warning
    
    // Test invalid pointer
    memfree((void *)0x12345678);  // Should print warning
#endif
    
    printf("=== Edge Cases Test Complete ===\n\n");
}

int main(void) {
    printf("Memory Allocator Implementation\n");
    printf("Heap size: %d bytes (%d KB)\n\n", HEAP_SIZE, HEAP_SIZE / 1024);
    
#ifdef TRIVIAL_ALLOCATOR
    printf("Using TRIVIAL ALLOCATOR (allocation only)\n");
    test_trivial_allocator();
#else
    printf("Using FREE LIST ALLOCATOR (full implementation)\n");
    test_free_list_allocator();
#endif
    
    test_edge_cases();
    
    printf("All tests completed!\n");
    return 0;
}
