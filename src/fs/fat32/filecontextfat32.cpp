#include "filecontextfat32.h"
#include "util/string.h"
#include "util/log.h"
#include "types/status.h"

kernel::fs::FileContextFAT32::FileContextFAT32(FAT32 &fs, string path)
    : fs(fs), path(path), sectorBuffer(nullptr), pos(0), lastSector(-1)
{
}

kernel::fs::FileContextFAT32::~FileContextFAT32()
{
    if (sectorBuffer != nullptr)
    {
        delete sectorBuffer;
    }
}

int kernel::fs::FileContextFAT32::read(void *buffer, int n)
{
    int filesize;
    if (fs.file_size(path, filesize) == failure)
    {
        kernelLog(LogLevel::ERROR, "Failed to obtain file size of %s during read.", path.c_str());
        return EIO;
    }

    if (pos >= filesize)
    {
        // kernelLog(LogLevel::DEBUG, "Hit EOF on %s during read.", path.c_str());
        return EEOF;
    }
    else if (pos + n > filesize)
    {
        n = filesize - pos;
    }

    unsigned int count = 0;
    while (count < n)
    {
        int sector = pos / fs.get_sector_size();
        int offset = pos % fs.get_sector_size();
        int bytesLeft = fs.get_sector_size() - offset;
        if (sector != lastSector)
        {
            if (sectorBuffer != nullptr)
            {
                delete sectorBuffer;
            }
            if (fs.read_file(path, sector, sectorBuffer) == failure)
            {
                kernelLog(LogLevel::WARNING, "I/O error reading sector %i of %s", sector, path.c_str());
                break;
            }
        }
        int c = (n > bytesLeft) ? bytesLeft : n;
        memcpy(buffer + count, sectorBuffer + offset, c);
        count += c;
        pos += c;
    }
    return count;
}

int kernel::fs::FileContextFAT32::write(const void *buffer, int n)
{
    return EIO;
}

kernel::fs::FileContext *kernel::fs::FileContextFAT32::copy()
{
    FileContextFAT32 *f = new FileContextFAT32(fs, path);
    if (f != nullptr)
    {
        f->pos = pos;
        f->fileSize = fileSize;
    }
    return f;
}
