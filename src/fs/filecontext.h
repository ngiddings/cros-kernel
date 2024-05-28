#ifndef KERNEL_FILECONTEXT_H
#define KERNEL_FILECONTEXT_H

#include "util/hasrefcount.h"

namespace kernel::fs
{
    class FileContext : public HasRefcount
    {
    public:
        virtual ~FileContext() = 0;

        virtual int read(void *buffer, int n) = 0;

        virtual int write(const void *buffer, int n) = 0;

        virtual FileContext *copy() = 0;
    };
}

#endif