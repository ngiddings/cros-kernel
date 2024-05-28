
#include "heap.h"
#include "mmap.h"
#include "pageallocator.h"
#include "util/string.h"
#include "../containers/math.h"
#include <cstdint>
#include "util/log.h"

unsigned long *heap;
unsigned long *linked_list_start = nullptr; // Actually last block in heap, but start of ll
unsigned long *search_start = nullptr;      // Set at start of search, don't maintain
unsigned long *search_previous = nullptr;   // Must maintain for search
unsigned long *search_current = nullptr;    // Must maintain for search
unsigned long *heap_end = nullptr;
// unsigned long num_free_blocks;

void init_heap(void *heap_ptr, unsigned long mem_size)
{
  linked_list_start = nullptr;
  search_start = nullptr;
  search_previous = nullptr;
  search_current = nullptr;
  int overhead_adj_size = mem_size - (WORD_SIZE * OVERHEAD);
  heap = (unsigned long *)heap_ptr;
  heap[0] = overhead_adj_size / SIZE_T_SIZE; // Header size var
  heap[1] = (unsigned long)nullptr;
  heap[2] = 0; // Header flag var
  heap[(mem_size / SIZE_T_SIZE) - 2] = heap[0];
  heap[(mem_size / SIZE_T_SIZE) - 1] = heap[1];
  linked_list_start = search_start = search_current = heap;
  // num_free_blocks = 1;
  heap_end = &heap[(mem_size / SIZE_T_SIZE)];
}

unsigned long next_power_of_2(unsigned long n)
{
  unsigned count = 0;
  // First n in the below condition
  // is for the case where n is 0
  if (n && !(n & (n - 1)))
  {
    return n;
  }
  while (n != 0)
  {
    n >>= 1;
    count += 1;
  }
  return 1 << count;
}

void expand_heap(unsigned long size)
{
  physaddr_t new_frame;
  unsigned long adj_size = next_power_of_2(size);
  if (size <= 4096)
  { // Min size
    new_frame = kernel::memory::pageAllocator.reserve(4096);
  }
  else
  {
    new_frame = kernel::memory::pageAllocator.reserve(adj_size);
  }
  kernel::memory::map_region(heap_end, adj_size, new_frame, kernel::memory::PAGE_RW);

  heap_end = heap_end + adj_size;
  rebuild_list(search_current);
}

unsigned long *split_blk(unsigned long size, unsigned long *blk_to_split)
{
  unsigned long *return_block = blk_to_split;
  unsigned long *split_block = blk_to_split + (size / SIZE_T_SIZE) + OVERHEAD; // Current address + size + overhead to get address of new block

  split_block[0] = blk_to_split[0] - (size / SIZE_T_SIZE) - OVERHEAD; // Set header size
  split_block[1] = blk_to_split[1];                                   // What ever blk_to_split points to, split block now points to
  split_block[2] = 0;                                                 // Flag
  split_block[split_block[0] + OVERHEAD - 2] = split_block[0];        // Set footer size
  split_block[split_block[0] + OVERHEAD - 1] = 0;                     // Set footer flag

  // Maintain Linked list pointers
  if (search_previous && search_previous[1])
  {
    search_previous[1] = (unsigned long)split_block;
  }
  if (blk_to_split == linked_list_start)
  {
    linked_list_start = split_block;
  }
  if (blk_to_split == search_previous)
  {
    search_previous = split_block;
  }
  if (blk_to_split == search_current)
  {
    search_current = split_block;
  }
  return_block[0] = size / SIZE_T_SIZE;                           // Set header size
  return_block[1] = 0;                                            // Reset next address to null
  return_block[2] = 1;                                            // Set flag
  return_block[return_block[0] + OVERHEAD - 2] = return_block[0]; // Setting footer size
  return_block[return_block[0] + OVERHEAD - 1] = 1;               // Setting footer flag

  return &return_block[3];
}

unsigned long *find_fit(unsigned long size)
{
  search_start = search_current;
  do
  {
    if ((search_current[0] * SIZE_T_SIZE) == size + OVERHEAD)
    { // Block is correct size, use it
      unsigned long *return_block = search_current;
      if (search_previous && search_current != search_start)
      {
        search_previous[1] = return_block[1]; // Update linked list, removing search next as it is now in use
      }
      search_current = (unsigned long *)return_block[1]; // Go to next block for future calls
      return_block[2] = 1;                               // Set flag
      return_block[return_block[0] + 2] = 1;             // Set footer flag
      return &return_block[3];                           // Return address of start of useable memory
    }
    else if ((search_current[0] * SIZE_T_SIZE) > ALIGN(size + ((OVERHEAD + 1) * SIZE_T_SIZE * 2)))
    { // Block is oversized, split
      return split_blk(size, search_current);
    }
    else if (search_current[1])
    { // Block is not right size and not end of linked list, iterate
      search_previous = search_current;
      search_current = (unsigned long *)search_current[1];
    }
    else
    { // End of list, go to start and iterate
      search_previous = search_current;
      search_current = linked_list_start;
    }
  } while (search_previous && search_start != search_current && search_current < heap_end);

  return nullptr; // Failed to find block big enough
}

void *rmalloc(unsigned long size)
{
  unsigned long blk_size = ALIGN(size);
  unsigned long *block = find_fit(blk_size);
  if (!block)
  { // Out of space, request more;
    expand_heap(size);
    block = find_fit(blk_size); // Try allocation again
    if (!block)
    {
      return nullptr; // Panic, mem expansion failed
    }
  }
  return block;
}

void rebuild_list(unsigned long *current)
{
  unsigned long *ll_cur, *ll_prev;
  ll_prev = linked_list_start; // First block in ll
  ll_cur = heap;               // First block in physical mem

  // Rebuild ll WATCHME High computation since traversing whole ll?
  while (ll_cur < heap_end)
  {
    while (ll_cur[2] != 0)
    {
      ll_cur = ll_cur + ll_cur[0] + OVERHEAD; // Start of next block
    }
    if (ll_cur != linked_list_start)
    {
      if (ll_cur == current)
      { // Set previous and current
        search_previous = ll_prev;
        search_current = ll_cur;
      }
      ll_prev[1] = (unsigned long)ll_cur;
      ll_prev = ll_cur;
      ll_cur = ll_cur + ll_cur[0] + OVERHEAD; // Start of next block
    }
    else
    {
      break; // End of heap
    }
  }
  if (current == linked_list_start && linked_list_start[1])
  { // Edge Case when linked_list_start is current, move current to next node
    search_previous = linked_list_start;
    search_current = (unsigned long *)linked_list_start[1];
  }
  ll_prev[1] = 0; // End of list, set next addr to 0 so list doesn't loop
}

void merge(unsigned long *current)
{
  unsigned long *previous, *next, *ll_cur, *ll_prev;
  previous = next = nullptr;

  if (current != heap)
  {
    if ((current - 1)[0] == 0)
    { // Previous block is free, merge
      previous = (current - ((current - 2)[0] + OVERHEAD));
    }
  }

  if ((current + current[0] + OVERHEAD + 2)[0] == 0 && (current + current[0] + OVERHEAD + 2) < heap_end)
  { // Next block is free, merge  NOTE 2 to get to the flag on the next block
    next = (current + current[0] + OVERHEAD);
  }

  if (previous)
  {                                                    // Merge previous
    previous[0] = previous[0] + current[0] + OVERHEAD; // Set size
    current[1] = 0;                                    // NOTE Not required, just cleans up heap for debugging

    if (current == linked_list_start)
    { // Need to maintain, otherwise rebuild will fail, all other pointers set in rebuild
      linked_list_start = previous;
    }

    if (current == search_current)
    {
      search_current = previous;
    }

    previous[previous[0] + OVERHEAD - 2] = previous[0]; // Set footer size, flag is already set
    current = previous;                                 // Set this incase next is also being merged
    // num_free_blocks--;
  }

  if (next)
  {                                               // Merge next
    current[0] = current[0] + next[0] + OVERHEAD; // Set size
    next[1] = 0;                                  // NOTE Not required, just cleans up heap
    if (next == linked_list_start)
    { // Need to maintain, otherwise rebuild will fail, all other pointers set in rebuild
      linked_list_start = current;
    }

    if (next == search_current)
    {
      search_current = current;
    }

    current[current[0] + OVERHEAD - 2] = current[0]; // Set footer size, flag is already set
    // num_free_blocks--;
  }

  if (previous || next)
  { // Only rebuild list if merge occured
    rebuild_list(current);
  }
}

void rfree(void *ptr)
{
  unsigned long *blk_free = (unsigned long *)ptr - 3; // Subtract 3 to get from start of data field to start of header
  blk_free[2] = 0;                                    // Set flag
  blk_free[blk_free[0] + OVERHEAD - 1] = 0;           // Set footer flag
  // Insert into linked list
  if (search_previous)
  {                                   // Already a list, insert
    blk_free[1] = search_previous[1]; // search_previous-> blk_free -> search_next
    search_previous[1] = (unsigned long)blk_free;
  }
  else
  { // No list, start one
    search_previous = linked_list_start;
    search_previous[1] = (unsigned long)blk_free;
  }
  search_current = blk_free;

  // num_free_blocks++;
  // Check for merging
  merge(blk_free);
}

void *realloc(void *ptr, unsigned long new_size)
{
  unsigned long *return_blk, *blk_old;
  int flag = 1;                       // Tries mem expansion twice before failing
  blk_old = (unsigned long *)ptr - 3; // Subtract 3 to get from start of data field to start of header

  if (!ptr)
  { // If pointer null, same as malloc
    return rmalloc(new_size);
  }
  else if (blk_old[0] > new_size + OVERHEAD + 1)
  { // Need to shrink and free FIXME Potential Fragmentation, merge not called on freed block
    return_blk = split_blk(new_size, blk_old);
    rebuild_list(return_blk);
    return return_blk;
  }
  // Need to expand, check if space in current heap
  do
  { // CLEANME
    return_blk = find_fit(new_size);
    if (return_blk)
    { // Found space in current heap, copy data;
      memcpy(return_blk, ptr, blk_old[0]);
    }
    // No mem left, expand
    expand_heap(new_size);
    flag--;
  } while (flag >= 0);

  return NULL; // Panic, expansion failed
}

/*Debug Fuctions*/

uint64_t xorshift64(uint64_t state) // Psuedo Random Number Generator
{
  uint64_t x = state;
  x ^= x << 13;
  x ^= x >> 7;
  x ^= x << 17;
  return state = x;
}

uint64_t randinrange(uint64_t lower, uint64_t upper, uint64_t rand)
{
  return (rand % (upper - lower + 1)) + lower;
}

void debug_bounds_check(unsigned long *a, unsigned long *b, uint64_t size_b)
{
  unsigned long *blk1 = a - 3;
  unsigned long *blk2 = b - 3;
  if (blk1 == blk2)
  {
    return;
  }
  if (blk1 >= blk2 && blk1 < blk2 + size_b)
  {
    while (1)
    {
    }
  }
}
