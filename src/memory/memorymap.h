/**
 * Author: Nathan Giddings
 */

#ifndef KERNEL_MEMORYMAP_H
#define KERNEL_MEMORYMAP_H

namespace kernel::memory
{

/**
 * @brief Describes in broad terms the layout of an address space 
 * (or parts(s) of an address space). Most useful for getting an initial
 * layout of the physical address space.
 */
class MemoryMap
{
public:

    /**
     * @brief Enumerator declaring the various memory types
     */
    enum class MemoryType
    {
        /**
         * @brief Memory at the described location is available for use as RAM.
         */
        AVAILABLE = 1,

        /**
         * @brief Memory at the described location is unavailable for use for
         * some unspecified reason.
         */
        UNAVAILABLE = 2,

        /**
         * @brief Memory at this region is used for MMIO and should not be used
         * as RAM.
         */
        MMIO = 3,

        /**
         * @brief Memory at the described region is defective and cannot be used.
         */
        DEFECTIVE = 4
    };

    /**
     * @brief Describes a particular region of memory starting at `location`
     * and extending `size` bytes, with its use indicated by `type`.
     */
    class MemoryRegion 
    {
        friend class MemoryMap;
    public:

        /**
         * @brief Default contructor
         */
        MemoryRegion();

        /**
         * @brief Contructs a MemoryRegion object
         * @param type 
         * @param location 
         * @param size 
         */
        MemoryRegion(MemoryType type, unsigned long location, unsigned long size);

        /**
         * @brief Compares the location and size of two MemoryRegions
         * @param rhs object to compare to
         * @return true if `rhs` should be placed after `lhs` in a MemoryMap.
         */
        bool operator<(const MemoryRegion& rhs) const;

        /**
         * @brief Compares the location and size of two MemoryRegions
         * @param rhs object to compare to
         * @return true if `rhs` should be placed before `lhs` in a MemoryMap.
         */
        bool operator>(const MemoryRegion& rhs) const;

        /**
         * @brief Checks if this object overlaps the region described by `rhs`
         * @param rhs object to compare to
         * @return true if the two regions overlap
         */
        bool overlaps(const MemoryRegion& rhs) const;

        /**
         * @brief Checks if this object completely contains the region 
         * described by `rhs`
         * 
         * @param rhs object to compare to
         * @return true if `rhs` is completely contained within this region
         */
        bool contains(const MemoryRegion& rhs) const;

        /**
         * @brief Computes the location at the end of this memory region,
         * given by (`location` + `size`).
         * 
         * @return The first location after this memory region.
         */
        unsigned long end() const;

        /**
         * @return An enumerator representing the type of memory in this region.
         */
        MemoryType getType() const;

        /**
         * @return The starting location of this region.
         */
        unsigned long getLocation() const;

    private:

        /**
         * @brief The type or use of the memory in this region. Higher values
         * overwrite lower values as the memory map is built.
         */
        MemoryType type;

        /**
         * @brief The location of the beginning of this memory region.
         */
        unsigned long location;

        /**
         * @brief The size of this memory region in bytes.
         */
        unsigned long size;

    };

    /**
     * @brief Default constructor.
     */
    MemoryMap();

    /**
     * @brief Places a new region into the memory map. Regions with higher type
     * values overwrite overlapping regions with lower type values. The map's
     * array is modified so that no two regions overlap, and the regions are
     * by location in ascending order.
     * 
     * @param type the type of new new region to place
     * @param location the location of the new region to place
     * @param size the size of the new region to place
     * @return Nonzero if there is insufficient room left in the map's internal
     * array.
     */
    int place(MemoryType type, unsigned long location, unsigned long size);

    /**
     * @brief Gets the size of the MemoryRegion array
     * @return the number of MemoryRegions inside this map.
     */
    int size() const;

    /**
     * @brief Access a particular entry in the memory map
     * @param index the index of the entry to access
     * @return a reference to the requested entry
     */
    const MemoryRegion& operator[](int index) const;

private:

    /**
     * @brief Maximum number of different regions that can be inside a memory map
     */
    static const unsigned long capacity = 16;

    /**
     * @brief Current number of regions inside this memory map. Any objects
     * in `map` at or after index `mapSize` will be uninitialized.
     */
    int mapSize;

    /**
     * @brief Array of memory regions describing the layout of an address space.
     */
    MemoryRegion map[capacity];

    /**
     * @brief Modifies the map array starting at `index` to ensure that
     * subsequent regions do not overlap, and that adjacent regions of the
     * same type are merged.
     * @param index the array index to start operation on
     * @return the next index at which this method should be called, or -1 if
     * the end of the array has been reached
     */
    int trim(int index);

    /**
     * @brief Removes a particular map entry from the array and moves
     * subsequent entries backwards one space.
     * 
     * @param index the index of the element to be removed
     */
    void remove(int index);

    /**
     * @brief Inserts a new region into the map such that the array remains
     * sorted in ascending order
     * 
     * @param type the type of the new region to insert
     * @param location the location of the new region to insert
     * @param size the size of the new region to insert
     */
    void insert(MemoryType type, unsigned long location, unsigned long size);

};

}

#endif