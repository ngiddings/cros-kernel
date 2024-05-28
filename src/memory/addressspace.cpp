#include "addressspace.h"

using namespace kernel::memory;

AddressSpace::AddressSpace(physaddr_t frame, int id)
    : topLevelTable(frame), id(id)
{
}

physaddr_t AddressSpace::getTableFrame() const
{
    return topLevelTable;
}

int AddressSpace::getId() const
{
    return id;
}
