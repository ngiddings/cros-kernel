#include "mmap.h"
#include "pageallocator.h"
#include "util/log.h"

using namespace kernel::memory;

int idCounter = 1;

AddressSpace *kernel::memory::createAddressSpace()
{
    physaddr_t frame = pageAllocator.reserve(getBlockSize(0));
    if (frame == PageAllocator::NOMEM)
    {
        return nullptr;
    }

    initializeTopTable(frame);
    AddressSpace *obj = new AddressSpace(frame, idCounter);
    idCounter++;
    return obj;
}

void kernel::memory::destoryAddressSpace(AddressSpace &addressSpace)
{
    // kernelLog(LogLevel::WARNING, "destroyAddressSpace(id = %i) called: not implemented.", addressSpace.getId());
    //  TODO
}

int kernel::memory::map_region(void *addr, size_t size, physaddr_t frame, int flags)
{
    // TODO: Make this more efficient. Map larger blocks when possible.
    for (unsigned long p = 0; p < size; p += getBlockSize(0))
    {
        setPageEntry(0, addr + p, frame + p, flags);
    }
    return 0;
}

physaddr_t kernel::memory::unmap_region(void *addr, size_t size)
{
    for (unsigned long p = 0; p < size; p += getBlockSize(0))
    {
        clearEntry(0, addr + p);
    }
    return 0;
}
