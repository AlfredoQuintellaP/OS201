# Structure d'un système de fichier et layout mémoire (TP1)

Still have to correct smll mstk to post

# Organisation de la mémoire (TP2)

## Project Overview
This project involves implementing a custom memory allocator from scratch, providing functionality similar to the standard C library's `malloc` and `free` functions.  
The goal is to understand how memory management works at the system level by building a heap allocator with both simple and advanced implementations.

## Learning Objectives
- Understand heap memory organization  
- Implement memory allocation and deallocation algorithms  
- Manage memory fragmentation and coalescing  
- Work with metadata management in memory blocks  
- Compare different allocation strategies (first-fit)  

---

## Project Structure
The project progresses through two main implementations:

### Phase 1: Trivial Allocator (Allocation Only)
- Simple memory allocation without deallocation support  
- Single contiguous free memory block  
- Suitable for one-time allocation scenarios  

### Phase 2: Free List Allocator (Complete Implementation)
- Full allocation and deallocation support  
- Free list management using linked lists  
- Metadata storage for block tracking  
- Fragmentation mitigation through coalescing  

---

## Technical Specifications

### Heap Configuration
- **Heap Size:** 16 KB statically allocated  
- **Memory States:** Each byte is either `FREE` or `ALLOCATED`  
- **No External Allocation:** Do not use `malloc`/`free` from the standard library  

---

## Free List Implementation Details

### Metadata Structure
Each memory block requires metadata stored at the beginning:
- Block size (including metadata)  
- Pointer to next free block (for free blocks)  

### Allocation Strategy: First-Fit
1. Traverse free list from the beginning  
2. Stop at the first block large enough to satisfy request  
3. Split blocks when remaining space is sufficient  
4. Return `NULL` when no suitable block is found  

### Deallocation and Coalescing
- Insert freed blocks back into free list  
- Merge adjacent free blocks to reduce fragmentation  
- Handle edge cases (double-free, invalid pointers)  

---

## Implementation Tasks

### Phase 1: Basic Allocator
1. Create 16KB static heap  
2. Implement trivial `memalloc()` (no deallocation)  

### Phase 2: Free List Allocator
3. Organize code with preprocessor directives  
4. Initialize heap with single free block  
5. Implement first-fit allocation  
6. Add metadata storage to allocation  
7. Implement memory deallocation  
8. Analyze fragmentation problems  
9. Implement basic coalescing (next-block only)  
10. Evaluate limitations and potential improvements  

---

## Key Concepts Explained

### Memory Fragmentation
Fragmentation occurs when free memory becomes divided into small, non-contiguous blocks, making it impossible to satisfy large allocation requests even when total free memory is sufficient.

### Coalescing Strategies
- **Simple Coalescing:** Merge with next adjacent free block only  
- **Advanced Coalescing:** Merge with both previous and next adjacent blocks  
- **Boundary Tags:** Store size information at both ends of blocks for bidirectional traversal  

### Allocation Algorithms Comparison
- **First-Fit:** Fast but can lead to external fragmentation  
- **Best-Fit:** Reduces fragmentation but slower  
- **Worst-Fit:** Better for large allocations but inefficient for small ones  

---

## Compilation and Usage
Use preprocessor directives to switch between implementations.

```c
#define TRIVIAL_ALLOCATOR  // Comment/uncomment to switch versions

#ifdef TRIVIAL_ALLOCATOR
    // Phase 1 implementation
#else
    // Phase 2 implementation (free list)
#endif
```

---

## Potential Improvements
- **Better Coalescing:** Implement bidirectional coalescing using boundary tags  
- **Allocation Strategies:** Add support for best-fit or worst-fit algorithms 

---

## To Compile and Run

```bash
# Compile version 1 
gcc -DTRIVIAL_ALLOCATOR -o allocator_trivial main.c

# Compile version 2
gcc -o allocator_free_list main.c

# For debbunging
gcc -g -DTRIVIAL_ALLOCATOR -o allocator_trivial_debug main.c
gcc -g -o allocator_free_list_debug main.c

# All warning actives
gcc -Wall -Wextra -DTRIVIAL_ALLOCATOR -o allocator_trivial main.c
gcc -Wall -Wextra -o allocator_free_list main.c

# Execute tests
./allocator_trivial
./allocator_free_list```

# Co-routines et threads coopératifs (TD3 et TD4) 

