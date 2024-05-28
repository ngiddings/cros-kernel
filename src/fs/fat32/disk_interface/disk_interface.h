#ifndef DISK_INTERFACE_H
#define DISK_INTERFACE_H

#include "../helpers.h"

class DiskInterface
{
public:
    DiskInterface(string disk_filepath, int bytes_per_sector);
    DiskInterface(void *ramfs, int bytes_per_sector);
    ~DiskInterface();

    byte *read(int sector_index);
    void write(int sector_index, const byte *buffer);

private:
    string disk_filepath;
    int bytes_per_sector;
    byte *disk;
    unsigned int size;

    void write_to_disk();
};

#endif