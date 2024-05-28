/**
 * Author: Nathan Giddings
 * 
 * Code adapted from personal project: https://github.com/ngiddings/quark-libmalloc/blob/master/include/libmalloc/buddy_alloc.h
 */
#ifndef PAGE_ALLOCATOR_H
#define PAGE_ALLOCATOR_H

#include "memorymap.h"
#include "types/physaddr.h"

namespace kernel::memory
{

/**
 * @brief Manages the reservation and freeing of physical memory pages.
 */
class PageAllocator
{
public:

    static const physaddr_t NOMEM = (physaddr_t)(~0);

    static unsigned long mapSize(MemoryMap& map, unsigned long blockSize);

    PageAllocator();

    /**
     * @brief Constructs the page allocator's internal data structures from an
     * existing description of the memory layout.
     * 
     * @param map a description of the physical memory layout
     * @param mapBase location
     * @param blockSize the size in bytes of a single block of memory
     */
    PageAllocator(MemoryMap& map, void *mapBase, unsigned long blockSize);

    /**
     * @brief Reserves a chunk of memory at least `size` bytes long. The
     * reserved physical memory will be contiguous; therefore, the allocator
     * can potentially fail due to fragmentation.
     * 
     * @param size the minimum number of bytes to reserve
     * @return the physical address of the newly allocated chunk of memory, or
     * NOMEM upon failure.
     */
    physaddr_t reserve(unsigned long size);

    /**
     * @brief 
     * @param location 
     * @return 
     */
    unsigned long free(physaddr_t location);

private:

    class Block
    {
    public:

        static const unsigned long RESERVED = 0;

        static const unsigned long FREE = 1;

        Block();
        
        Block(Block *linkf, Block *linkb, unsigned long kval, unsigned long tag);

        Block *linkb;

        Block *linkf;

        unsigned long kval;

        unsigned long tag;

    };

    /**
     * @brief The number of distinct block sizes that are supported. Serves
     * as the maximum possible value of `maxKVal`, and defines the size of
     * the largest possible block: (2 ^ (availListSize-1)) * blockSize.
     */
    static const int availListSize = 32;

    Block availList[availListSize];

    Block *blockMap;

    unsigned long blockMapSize;

    unsigned long maxKVal;

    unsigned long blockSize;

    unsigned long offset;

    unsigned long freeBlockCount;

    /**
     * @brief Inserts a new block into the appropriate linked list, performing
     * mergers with buddy blocks as needed.
     * 
     * @param index 
     * @param k 
     */
    void insert(unsigned long index, unsigned long k);

};

extern PageAllocator pageAllocator;

}

#endif