#ifndef KERNEL_MMAP_H
#define KERNEL_MMAP_H

#include "addressspace.h"
#include "types/physaddr.h"
#include <cstddef>

namespace kernel::memory
{

    /**
     * @brief Size in bytes of a single page
     */
    extern const unsigned long page_size;

    /**
     * @brief Permission flags for controlling memory access at page level
     */
    enum PageFlags
    {
        /**
         * @brief When set, page is writable. When clear, page is read-only.
         */
        PAGE_RW = (1 << 0),

        /**
         * @brief When set, page is accesable from usermode. When clear, page is
         * only accessable in kernelmode.
         */
        PAGE_USER = (1 << 1),

        /**
         * @brief When set, page can contain executable code. When clear, code
         * in this page cannot be executed.
         */
        PAGE_EXE = (1 << 2)
    };

    /**
     * @brief Creates and initialzes a new address space, the constructs a
     * corresponding AddressSpace object.
     *
     * @return a pointer to a newly allocated AddressSpace object
     */
    AddressSpace *createAddressSpace();

    /**
     * @brief Frees all physical memory associated with the given address space.
     * The address space will no longer be usable, and the caller is expected to
     * delete the relevant AddressSpace object.
     *
     * @param addressSpace object describing the address space to destory.
     */
    void destoryAddressSpace(AddressSpace &addressSpace);

    /**
     * @brief Switches to the provided address space. Calls to `map_region` and
     * other such functions will modify only the currently active address space
     * (details on this may be platform specific).
     *
     * @param addressSpace address space to load
     */
    void loadAddressSpace(AddressSpace &addressSpace);

    /**
     * @brief Map the region of memory starting at `addr` to frames starting at `frame`.
     * @param addr linear address of the region to map
     * @param size size in bytes of the region to map (should be multiple of page size)
     * @param frame starting physical address to map this region to
     * @param flags permission flags for the pages to map
     * @return 0 upon success, nonzero upon failure
     */
    int map_region(void *addr, size_t size, physaddr_t frame, int flags);

    /**
     * @brief Unmap the region of memory starting at `addr`. Does not free any
     * frames; this must be done by caller.
     *
     * @param addr Starting address of region to unmap
     * @param size Size in bytes of region to unmap
     * @return Frame pointed to by `addr`
     */
    physaddr_t unmap_region(void *addr, size_t size);

    /**
     * @brief Initializes the given frame as a top-level translation table.
     *
     * Implementation of this function is platform-dependent.
     *
     * @param frame Frame to store new top table in
     */
    void initializeTopTable(physaddr_t frame);

    /**
     * @brief Gets the size of a page or block at the given level. Level 0 is
     * the smallest available granularity.
     *
     * Implementation of this function is platform-dependent.
     *
     * @param level Translation level to use
     * @return Size in bytes of block at given level
     */
    size_t getBlockSize(int level);

    /**
     * @brief Gets the frame referenced by virtual address `page`, if it exists.
     *
     * Implementation of this function is platform-dependent.
     *
     * @param page Virtual address for with to get corresponding frame
     * @return Frame pointed to by `page`, or 0 if no frame exists.
     */
    physaddr_t getPageFrame(void *page);

    /**
     * @brief Writes a new page entry for the given page, pointing to the given
     * frame. The size of the page is implied by `level`.
     *
     * Implementation of this function is platform-dependent.
     *
     * @param level Translation level to use
     * @param page Virtual address to map
     * @param frame Frame to point the new page to
     * @param flags Permission flags for the new page
     */
    void setPageEntry(int level, void *page, physaddr_t frame, int flags);

    /**
     * @brief Writes a new translation table entry for the virtual memory region
     * starting at `page`. The new table is backed by `table`.
     *
     * Implementation of this function is platform-dependent.
     *
     * @param level Translation level of the new table entry. 0 is invalid, as it
     * is always the final translation level.
     * @param page Virtual address for the new table entry
     * @param table Frame to store the new translation table
     */
    void setTableEntry(int level, void *page, physaddr_t table);

    /**
     * @brief Clears the translation table entry corresponding to this location,
     * and marks it as 'not present'.
     *
     * @param level Translation level of the entry to clear
     * @param page Virtual address of the page corresponding to the entry to clear
     */
    void clearEntry(int level, void *page);

}

#endif