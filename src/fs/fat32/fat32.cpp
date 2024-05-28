/*
 * Author: Hunter Overstake
 * Email: huntstake@gmail.com
 * Date: 5/14/2024
 */

#include "fat32.h"
#include "util/log.h"

/* Constructs the file system tree in memeory to keep track of meta-data */
FAT32::FAT32(string disk_filepath)
{
    di = new DiskInterface(disk_filepath, 512);

    // read and save required boot sector info
    byte *buffer = di->read(0);
    init(buffer);
    delete[] buffer;
}

FAT32::FAT32(void *ramfs)
{
    di = new DiskInterface(ramfs, 512);
    // read and save required boot sector info
    byte *buffer = di->read(0);
    init(buffer);
    delete[] buffer;
}

// TODO
FAT32::~FAT32()
{
    // will segfaults if you uncomment

    // delete root_dir;
    // delete di;
}

void FAT32::init(byte *root_sector)
{
    bytes_per_sector = bytes_to_short(root_sector, 11);
    sectors_per_cluster = root_sector[13];
    reserved_sectors = bytes_to_short(root_sector, 14);
    fat_count = root_sector[16];
    total_sectors = bytes_to_int(root_sector, 32);
    if (total_sectors == 0)
    {
        total_sectors = bytes_to_short(root_sector, 19);
    }
    sectors_per_fat = bytes_to_int(root_sector, 36);

    bytes_per_fat_entry = sizeof(unsigned int);
    bytes_per_dir_entry = 32;
    bytes_per_cluster = bytes_per_sector * sectors_per_cluster;

    data_region_start_cluster_offset = (reserved_sectors + (fat_count * sectors_per_fat)) / sectors_per_cluster;
    reserved_fat_entries = 2;

    // start recursive search of the filesystem

    root_dir = new Entry(this, nullptr, data_region_start_cluster_offset * bytes_per_cluster);
}

/* Finds the file specified by the given file path and fills the given type parameter
with the type of the file (file or directory), will fail if it can't find the file. */
int FAT32::file_type(string file_path, FileType &type)
{
    Entry *entry = find_entry(file_path);

    if (entry == nullptr)
        return failure; // couldn't find file

    type = (FileType)entry->type;

    return success;
}

/* Finds the file specified by the given file path and fills the given size parameter
with the size of the file, will fail if it can't find the file. */
int FAT32::file_size(string file_path, int &size)
{
    Entry *entry = find_entry(file_path);

    if (entry == nullptr)
        return failure; // couldn't find file

    size = entry->file_size_bytes;

    return success;
}

/* Finds the file specified by the given file path and fills the given buffer parameter
with the byte data of the file at the given sector offset (using zero-indexing), will fail
if it can't find the file, or if the sector offset is bigger than the number of sectors the
file has. */
int FAT32::read_file(string file_path, int sector_offset, byte *&buffer)
{
    buffer = nullptr;

    Entry *entry = find_entry(file_path);

    if (entry == nullptr)
        return failure; // couldn't find file

    buffer = entry->read_data(sector_offset);

    if (buffer == nullptr)
        return failure; // couldn't read data

    return success;
}

/* Finds the file specified by the given file path and writes the byte data at the given sector
offset (using zero-indexing) into the provided data buffer, will allocate space for the buffer
so don't allocate space for the buffer before passing, will fail if: it can't find the file,
there isn't enough space left on disk to allocate more space for the write. */
int FAT32::write_file(string file_path, int sector_offset, const byte *data)
{
    Entry *entry = find_entry(file_path);

    if (entry == nullptr)
        return failure; // couldn't find file

    if (!entry->write_data(sector_offset, data))
        return failure; // couldn't write data

    return success;
}

/* Will create a new file at the given file path, will fail if: file path doesn't exist, the parent
directory isn't a directory or, file name already exits in parent directory, there isn't enough space
left on disk to allocate more space for a new file entry. */
int FAT32::create_file(string file_path)
{
    string par_dir_path = snip_file_path(file_path);
    Entry *par_dir = find_entry(par_dir_path);

    if (par_dir == nullptr)
        return failure; // invalid file path

    if (par_dir->type != EntryAttribute::DIRECTORY)
        return failure; // parent 'directory' isnt a directory

    Entry *existing_entry = find_entry(file_path);

    if (existing_entry != nullptr)
        return failure; // file with name already exists in directory

    Entry *new_entry = new Entry();
    string name = parse_file_path(file_path).back();
    if (!new_entry->new_dir_entry(par_dir, name, FILE))
    {
        delete new_entry;
        return failure; // not enough space in data region
    }

    return success;
}

// TODO
int FAT32::remove_file(string file_name)
{
    // TODO
    return failure;
}

// TODO
int FAT32::copy_file(string file_name, string dest_dir)
{
    // TODO
    return failure;
}

// TODO
int FAT32::move_file(string file_name, string dest_dir)
{
    // TODO
    return failure;
}

/* Finds the directory specified by the given file path and fill the given list with the directory's
sub-directory names, the given list will be emptied upon being passed, will fail if: it can't find the
directory, the 'directory' isn't a directory.*/
int FAT32::list_dir(string dir_path, vector<string> &list)
{
    list.clear();

    Entry *entry = find_entry(dir_path);

    if (entry == nullptr)
        return failure; // couldn't find directory

    if (entry->type != EntryAttribute::DIRECTORY)
        return failure; // 'directory' isnt a directory

    for (int i = 0; i < (int)entry->sub_dirs.size(); i++)
    {
        list.push_back(entry->sub_dirs[i]->name);
    }

    return success;
}

/* Will create a new directory at the given file path, will fail if: file path doesn't exist, the parent
directory isn't a directory or, file name already exits in parent directory, there isn't enough space
left on disk to allocate more space for a new directory entry. */
int FAT32::create_dir(string dir_path)
{
    string par_dir_path = snip_file_path(dir_path);
    Entry *par_dir = find_entry(par_dir_path);

    if (par_dir == nullptr)
        return failure; // invalid file path

    Entry *existing_entry = find_entry(dir_path);

    if (existing_entry != nullptr)
        return failure; // directory with name already exists in parent directory

    Entry *new_entry = new Entry();
    string name = parse_file_path(dir_path).back();
    if (!new_entry->new_dir_entry(par_dir, name, DIRECTORY))
    {
        delete new_entry;
        return failure; // not enough space in data region
    }

    return success;
}

// TODO
int FAT32::remove_dir(string name)
{
    // TODO
    return failure;
}

// TODO
int FAT32::copy_dir(string file_name, string dest_dir)
{
    // TODO
    return failure;
}

// TODO
int FAT32::move_dir(string file_name, string dest_dir)
{
    // TODO
    return failure;
}

int FAT32::get_sector_size()
{
    return bytes_per_sector;
}
