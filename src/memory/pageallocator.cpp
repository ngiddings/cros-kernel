#include "pageallocator.h"
#include "util/math.h"

using namespace kernel::memory;

PageAllocator kernel::memory::pageAllocator;

unsigned long PageAllocator::mapSize(MemoryMap &map, unsigned long blockSize)
{
    int index = map.size() - 1;
    while (map[index].getType() != MemoryMap::MemoryType::AVAILABLE)
    {
        index--;
    }

    return 1UL << llog2(sizeof(PageAllocator::Block) * map[index].end() / blockSize);
}

PageAllocator::PageAllocator()
{
    this->blockMap = nullptr;
    this->blockMapSize = 0;
    this->maxKVal = 0;
    this->blockSize = 0;
    this->offset = 0;
    this->freeBlockCount = 0;
}

PageAllocator::PageAllocator(MemoryMap &map, void *mapBase, unsigned long blockSize)
{
    this->blockMap = (Block *)mapBase;
    this->blockSize = blockSize;
    this->blockMapSize = mapSize(map, blockSize);
    this->offset = 0;
    this->freeBlockCount = 0;
    this->maxKVal = llog2(blockMapSize / sizeof(Block));
    for (int i = 0; i <= maxKVal; i++)
    {
        availList[i].linkf = &availList[i];
        availList[i].linkb = &availList[i];
    }

    for (int i = 0; i < blockMapSize / sizeof(Block); i++)
    {
        blockMap[i] = Block(nullptr, nullptr, 0, Block::RESERVED);
    }

    for (int i = 0; i < map.size(); i++)
    {
        if (map[i].getType() != MemoryMap::MemoryType::AVAILABLE)
        {
            continue;
        }

        unsigned long location = map[i].getLocation() + blockSize - 1;
        location -= location % blockSize;
        while (location + blockSize <= map[i].end())
        {
            insert(location / blockSize, 0);
            location += blockSize;
            freeBlockCount++;
        }
    }
}

physaddr_t PageAllocator::reserve(unsigned long size)
{
    unsigned long k = llog2((size - 1) / blockSize + 1);
    for (unsigned long j = k; j <= maxKVal; j++)
    {
        if (availList[j].linkf != &availList[j])
        {
            Block *block = availList[j].linkb;
            availList[j].linkb = block->linkb;
            availList[j].linkb->linkf = &availList[j];
            block->tag = Block::RESERVED;
            while (j > k)
            {
                j--;
                Block *buddy = block + (1UL << j);
                *buddy = Block(&availList[j], &availList[j], j, Block::FREE);
                block->kval = j;
                availList[j].linkb = buddy;
                availList[j].linkf = buddy;
            }
            unsigned long index = block - blockMap;
            freeBlockCount -= 1UL << k;
            return offset + index * blockSize;
        }
    }
    return NOMEM;
}

unsigned long PageAllocator::free(physaddr_t location)
{
    unsigned long index = (location - offset) / blockSize;
    unsigned long k = blockMap[index].kval;
    insert(index, k);
    return (1UL << k) * blockSize;
}

void PageAllocator::insert(unsigned long index, unsigned long k)
{
    freeBlockCount += 1UL << k;
    while (k < maxKVal)
    {
        unsigned long buddyIndex = index ^ (1UL << k);
        if (blockMap[buddyIndex].tag != Block::FREE || blockMap[buddyIndex].kval != k)
        {
            break;
        }
        blockMap[buddyIndex].linkb->linkf = blockMap[buddyIndex].linkf;
        blockMap[buddyIndex].linkf->linkb = blockMap[buddyIndex].linkb;
        blockMap[buddyIndex].tag = Block::RESERVED;
        k++;
        if (buddyIndex < index)
        {
            index = buddyIndex;
        }
    }
    Block *p = availList[k].linkf;
    blockMap[index] = Block(p, &availList[k], k, Block::FREE);
    p->linkb = &blockMap[index];
    availList[k].linkf = &blockMap[index];
}

PageAllocator::Block::Block()
    : linkf(nullptr), linkb(nullptr), kval(0), tag(RESERVED)
{
}

PageAllocator::Block::Block(Block *linkf, Block *linkb, unsigned long kval, unsigned long tag)
    : linkf(linkf), linkb(linkb), kval(kval), tag(tag)
{
}
