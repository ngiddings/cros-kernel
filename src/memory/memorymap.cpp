/**
 * Author: Nathan Giddings
 */

#include "memorymap.h"

using namespace kernel::memory;

MemoryMap::MemoryMap() : mapSize(0)
{}

int MemoryMap::place(MemoryType type, unsigned long location, unsigned long size)
{
    if(mapSize <= capacity - 2)
    {
        insert(type, location, size);
        int i = 0;
        while(i >= 0)
        {
            i = trim(i);
        }
        return 0;
    }
    else
    {
        return -1;
    }
}

int MemoryMap::size() const
{
    return mapSize;
}

const MemoryMap::MemoryRegion& MemoryMap::operator[](int index) const
{
    return map[index];
}

int MemoryMap::trim(int index)
{
    if(index + 1 >= mapSize)
    {
        return -1;
    }
    MemoryRegion& left = map[index];
    MemoryRegion& right = map[index + 1];
    if(left.overlaps(right) && left.type == right.type)
    {
        left.size = (right.end() > left.end() ? right.end() : left.end()) - left.location;
        remove(index + 1);
        return index;
    }
    else if(left.overlaps(right) && left.type < right.type && right.contains(left))
    {
        remove(index);
        return index;
    }
    else if(left.overlaps(right) && left.type < right.type && left.end() <= right.end())
    {
        left.size = (right.location > left.location) ? right.location - left.location : 0;
        return index + 1;
    }
    else if(left.overlaps(right) && left.type < right.type)
    {
        MemoryRegion newRight(left.type, right.end(), left.end() - right.end());
        left.size = (right.location > left.location) ? right.location - left.location : 0;
        if (left.size == 0)
        {
            remove(index);
        } 
        insert(newRight.type, newRight.location, newRight.size);
        return index + 2;
    }
    else if(left.overlaps(right) && left.contains(right))
    {
        remove(index + 1);
        return index;
    }
    else if(left.overlaps(right))
    {
        right.size = right.end() - left.end();
        right.location = left.end();
        return index + 1;
    }
    else if((left.end() == right.location) && left.type == right.type)
    {
        left.size = right.end() - left.location;
        remove(index + 1);
        return index;
    }
    else
    {
        return index + 1;
    }
}

void MemoryMap::remove(int index)
{
    if(index >= 0 && index < mapSize)
    {
        for(int i = index; i < mapSize - 1; i++)
        {
            map[i] = map[i + 1];
        }
        mapSize--;
    }
}

void MemoryMap::insert(MemoryType type, unsigned long location, unsigned long size)
{
    MemoryRegion newRegion(type, location, size);
    unsigned int i = 0;
    while(i < mapSize)
    {
        if(newRegion < map[i])
        {
            MemoryRegion buffer = newRegion;
            newRegion = map[i];
            map[i] = buffer;
        }
        i++;
    }
    map[i] = newRegion;
    mapSize++;
}

MemoryMap::MemoryRegion::MemoryRegion()
    : type(MemoryType::UNAVAILABLE), location(0), size(0)
{}

MemoryMap::MemoryRegion::MemoryRegion(MemoryType type, unsigned long location, unsigned long size)
    : type(type), location(location), size(size)
{}

bool MemoryMap::MemoryRegion::operator<(const MemoryRegion& rhs) const
{
    if(location == rhs.location)
    {
        return size < rhs.size;
    }
    else
    {
        return location < rhs.location;
    }
}

bool MemoryMap::MemoryRegion::operator>(const MemoryRegion& rhs) const
{
    if(location == rhs.location)
    {
        return size > rhs.size;
    }
    else
    {
        return location > rhs.location;
    }
}

bool MemoryMap::MemoryRegion::overlaps(const MemoryRegion& rhs) const
{
    if(rhs.location < location)
    {
        return rhs.end() > location;
    }
    else
    {
        return rhs.location < end();
    }
}

bool MemoryMap::MemoryRegion::contains(const MemoryRegion& rhs) const
{
    return (rhs.location >= location) && (rhs.end() <= end());
}

unsigned long MemoryMap::MemoryRegion::end() const
{
    return location + size;
}

MemoryMap::MemoryType MemoryMap::MemoryRegion::getType() const
{
    return type;
}

unsigned long MemoryMap::MemoryRegion::getLocation() const
{
    return location;
}