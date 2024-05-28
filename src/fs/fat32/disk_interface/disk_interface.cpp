#include "disk_interface.h"
/*#include <filesystem>
#include <fstream>
#include <iostream>*/
#include "containers/string.h"
#include "util/log.h"

DiskInterface::DiskInterface(string disk_filepath, int bytes_per_sector)
    : disk_filepath(disk_filepath), bytes_per_sector(bytes_per_sector)
{
    /*std::ifstream file(std::string(string::strdup(disk_filepath.c_str())), std::ios::binary);
    if (!file.is_open())
    {
        throw std::runtime_error("Error opening disk file");
    }

    file.seekg(0, std::ios::end);
    std::streampos file_size = file.tellg();
    file.seekg(0, std::ios::beg);

    size = static_cast<unsigned int>(file_size);
    disk = new byte[size];

    file.read(reinterpret_cast<char *>(disk), size);

    if (!file)
    {
        throw std::runtime_error("Error reading disk file");
    }

    file.close();*/
}

DiskInterface::DiskInterface(void *ramfs, int bytes_per_sector)
    : disk((byte *)ramfs), bytes_per_sector(bytes_per_sector), disk_filepath("")
{
}

DiskInterface ::~DiskInterface()
{
    write_to_disk();
    delete[] disk;
}

// must return a copy of the data cuz the buffer may be modified
byte *DiskInterface::read(int sector_index)
{
    byte *buffer = new byte[bytes_per_sector];

    int offset = bytes_per_sector * sector_index;
    for (int i = 0; i < bytes_per_sector; i++)
        buffer[i] = disk[i + offset];

    return buffer;
}

void DiskInterface::write(int sector_index, const byte *buffer)
{
    int offset = bytes_per_sector * sector_index;
    for (int i = 0; i < bytes_per_sector; i++)
        disk[i + offset] = buffer[i];

    return;
}

void DiskInterface::write_to_disk()
{
    /*disk_filepath = "disk_modified.img"; // to avoid disk containmination
    std::ofstream file(std::string(string::strdup(disk_filepath.c_str())), std::ios::binary);
    if (!file.is_open())
    {
        throw std::runtime_error("Error opening disk file for writing");
    }

    file.write(reinterpret_cast<char *>(disk), size);

    if (!file)
    {
        throw std::runtime_error("Error writing to disk file");
    }
    file.close();*/
}
