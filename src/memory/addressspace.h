#ifndef KERNEL_ADDRESSSPACE_H
#define KERNEL_ADDRESSSPACE_H

#include "util/hasrefcount.h"
#include "types/physaddr.h"

namespace kernel::memory
{

    class AddressSpace : public HasRefcount
    {
    public:
        AddressSpace(physaddr_t frame, int id);

        physaddr_t getTableFrame() const;

        int getId() const;

    private:
        physaddr_t topLevelTable;

        int id;
    };

};

#endif