/*
Author: Sam Lane

*/

#ifndef _KERNEL_HEAP_H
#define _KERNEL_HEAP_H

#include <cstdint>

#define WORD_SIZE sizeof(unsigned long) // Either 4 or 8, we're 64 bit so 8
#define OVERHEAD 5
#define ALIGN(size) (((size) + (WORD_SIZE - 1)) & ~(WORD_SIZE - 1))
#define SIZE_T_SIZE (ALIGN(sizeof(unsigned long))) // header size

/**
 * @brief Initialize heap at the memory address pointed to by  
 * heap_ptr of size mem_size. Sets initial block up with flags
 * and all pointers used for traversing memory.
 * 
 * @param heap_ptr 
 * @param mem_size 
 */
void init_heap(void *heap_ptr, unsigned long mem_size);

/**
 * @brief Expands avaliable heap memory by allocating and mapping
 * additional pages. 
 * 
 * @param size 
 */
void expand_heap(unsigned long size);

/**
 * @brief Iterates through all of memory, adding free blocks
 * to the linked list for later allocation.
 * 
 * @param current 
 */
void rebuild_list(unsigned long *current);

/**
 * @brief Checks previous and next blocks of memory to see
 * if they are free. If they are, a single block is created
 * from them and rebuild list is called to fix the linked
 * list pointers.
 * 
 * @param current 
 */
void merge(unsigned long *current);

/**
 * @brief Frees memory by setting the free bit flag and calling
 * merge to check surrounding blocks of memory. Ptr is the pointer
 * to writable memory, so it is subtracted by 3 to get to the start
 * of the memory block header. 
 * 
 * @param ptr 
 */
void rfree(void *ptr);

/**
 * @brief Allocates a memory block of size +5 for the header and footer
 * info. The header bytes are, in order, size of the block in bytes, pointer to
 * the next free block if in linked list, and in use flag. The footer
 * bytes are, in order, size of block in bytes and in use flag. Returns a ptr
 * to the user of the fourth byte in the allocated block, which is the first
 * free byte of memory after the header. 
 * 
 * @param size 
 * @return void* 
 */
void *rmalloc(unsigned long size);

/**
 * @brief Takes a pointer to allocated memory. If pointer is null, operates the
 * same as malloc. Otherwise, it either shrinks the allocated memory to fit in 
 * the new size, copying all memory up to the new size. If new size is larger, 
 * the heap is checked for a block of sufficient size, which is then allocated 
 * and the memory copied. If no block of sufficient size is found, memory is 
 * expanded and then the allocation is made. 
 * 
 * @param ptr 
 * @param new_size 
 * @return void* 
 */
void *realloc(void *ptr, unsigned long new_size);

/**
 * @brief Helper function used when expanding memory. Finds the next power
 * of 2 larger than n and returns it. 
 * 
 * @param n 
 * @return unsigned long 
 */
unsigned long next_power_of_2(unsigned long n);

/**
 * @brief Takes a large block of memory and splits it into two smaller
 * blocks, one of size size, and the other with the remaining size minus
 * the overhead for the header and footers. The block of size size is 
 * returned
 * 
 * @param size 
 * @param blk_to_split 
 * @return unsigned long* 
 */
unsigned long *split_blk(unsigned long size, unsigned long *blk_to_split);

/**
 * @brief Find fit traverses memory looking for free blocks of sufficient 
 * size. If a block is oversized, it is split into two via split_blk. 
 * Find_fit iterates over all free blocks of memory in the linked list 
 * until the starting free block is reached. It then fails to find a block
 * of sufficient size and returns a nullptr.
 * 
 * @param size 
 * @return unsigned long* 
 */
unsigned long *find_fit(unsigned long size);



// Testing functions KILLME
#define SIZE 30

void debug_bounds_check(unsigned long *a, unsigned long *b, uint64_t size_b);
uint64_t xorshift64(uint64_t state);
uint64_t randinrange(uint64_t lower, uint64_t upper, uint64_t rand);

struct test_struct
{
    unsigned long *pointer;
    int size;
};

#endif