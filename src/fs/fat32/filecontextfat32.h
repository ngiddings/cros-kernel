#ifndef KERNEL_FILECONTEXTFAT32_H
#define KERNEL_FILECONTEXTFAT32_H

#include "../filecontext.h"
#include "fat32.h"
#include "containers/string.h"

namespace kernel::fs
{
    class FileContextFAT32 : public FileContext
    {
    public:
        FileContextFAT32(FAT32 &fs, string path);

        ~FileContextFAT32();

        int read(void *buffer, int n);

        int write(const void *buffer, int n);

        FileContext *copy();

    private:
        FAT32 &fs;

        string path;

        byte *sectorBuffer;

        unsigned int fileSize;

        unsigned int pos;

        int lastSector;
    };
}

#endif