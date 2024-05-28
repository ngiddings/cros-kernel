/**
 * Author: Nathan Giddings
 */

#include "elf.h"
#include "memory/mmap.h"
#include "memory/pageallocator.h"
#include "util/string.h"

kernel::loader::ELF::ELF(void *file)
{
    header = (ELFFileHeader *)file;
    sectionIndex = 0;
}

const void *kernel::loader::ELF::fileLocation() const
{
    return (void *)header;
}

bool kernel::loader::ELF::isValid() const
{
    return header->magic == 0x464c457f && header->phcount > 0;
}

const kernel::loader::ELFFileHeader &kernel::loader::ELF::fileHeader() const
{
    return *header;
}

const kernel::loader::ELFProgramHeader &kernel::loader::ELF::currentSection() const
{
    ELFProgramHeader *sectionHeader = (ELFProgramHeader *)((void *)header + header->phoffset);
    return sectionHeader[sectionIndex];
}

bool kernel::loader::ELF::nextSection()
{
    sectionIndex++;
    if (sectionIndex >= header->phcount)
    {
        sectionIndex = 0;
    }
    return (sectionIndex > 0);
}

int kernel::loader::buildProgramImage(ELF &elf)
{
    using namespace kernel::memory;

    // Abort if binary isn't valid format
    if (!elf.isValid())
    {
        return -1;
    }

    do
    {
        // Check if section should be loaded
        if ((ELFSegmentType)elf.currentSection().type != ELFSegmentType::LOAD)
        {
            continue;
        }

        // Try to allocate enough physical memory
        unsigned long sectionSize = elf.currentSection().memsize;
        physaddr_t frame = pageAllocator.reserve(sectionSize);
        if (frame == PageAllocator::NOMEM)
        {
            return -1;
        }

        unsigned long fileSize = elf.currentSection().filesize;

        // Map virtual memory, then copy section data
        void *dest = elf.currentSection().vaddr;
        const void *src = elf.fileLocation() + elf.currentSection().offset;
        unsigned long diff = (unsigned long)dest % page_size;
        void *mapPtr = getPageFrame(dest) == 0 ? (dest - diff) : (dest + (page_size - diff));
        unsigned long mapSize = getPageFrame(dest) == 0 ? (sectionSize + diff) : (sectionSize - (page_size - diff));
        map_region(mapPtr, mapSize, frame, PAGE_RW | PAGE_USER | PAGE_EXE);
        memset(dest, sectionSize, 0);
        memcpy(dest, src, fileSize);
    } while (elf.nextSection());

    return 0;
}
