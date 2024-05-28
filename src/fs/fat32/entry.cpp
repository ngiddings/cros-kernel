/*
 * Author: Hunter Overstake
 * Email: huntstake@gmail.com
 * Date: 5/14/2024
 */

#include "fat32.h"

/* Creates an entry in memory for the FAT32 entry at the given byte offset to keep track of meta-data */
FAT32::Entry::Entry(FAT32 *fs, Entry *parent_dir, int byte_offset)
    : byte_offset(byte_offset), parent_dir(parent_dir), fs(fs)
{
    // figure out what type of entry this is
    // then parse accordingly
    type = parse_attribute(byte_offset);
    switch (type)
    {
    case ROOT:
        parse_root_entry();
        break;
    case DIRECTORY:
    case FILE:
        parse_dir_entry(); // similar enough to do in a single function
        break;
    case LONG_NAME:
        parse_long_name();
        break;
    default: // must be EMPTY
        clear_entry(byte_offset);
        break;
    }
}

// TODO
FAT32::Entry::~Entry()
{
    // will segfault
    for (int i = 0; i < sub_dirs.size(); i++)
        delete sub_dirs[i];

    for (int i = 0; i < long_name_entries.size(); i++)
        delete sub_dirs[i];
}

/* Reads the byte data of the entry at the specifed sector offset, will fail by returning null
if: entry isn't a file (e.g. a directory), the sector offset is bigger that the number of sectors
allocated to the file. */
byte *FAT32::Entry::read_data(int sector_offset)
{
    if (type != FILE)
        return nullptr; // entries not of type FILE don't have readable data

    int target_cluster = sector_offset / fs->sectors_per_cluster;

    if (target_cluster >= (int)fat_allocations.size())
        return nullptr; // can't read data that isn't allocated to the entry

    int target_cluster_offset = fat_entry_to_cluster_offset(fat_allocations[target_cluster]);
    int target_sector_offset = (target_cluster_offset * fs->sectors_per_cluster) + (sector_offset % fs->sectors_per_cluster);

    return fs->di->read(target_sector_offset);
}

/* Writes the given byte data to the given sector offset (using zero-indexing) into the provided data
buffer, will fail if: the entry isn't a file (e.g. a directory), there isn't enough space left on disk
to allocate more clusters if required. */
int FAT32::Entry::write_data(int sector_offset, const byte *data)
{
    if (type != FILE)
        return failure; // entries not of type FILE can't be written do

    int target_cluster = sector_offset / fs->sectors_per_cluster; // cluster the sector is in

    int num_clusters = (int)fat_allocations.size();
    if (target_cluster >= num_clusters)
    {
        int num_additional_clusters = (target_cluster + 1) - num_clusters; // need to allocate more clusters
        if (!allocate_more_clusters(num_additional_clusters))
            return failure; // couldn't allocate more clusters, data region must be full
    }

    int target_cluster_offset = fat_entry_to_cluster_offset(fat_allocations[target_cluster]);
    int target_sector_offset = (target_cluster_offset * fs->sectors_per_cluster) + (sector_offset % fs->sectors_per_cluster);

    fs->di->write(target_sector_offset, data); // write data

    unsigned int required_size = (sector_offset + 1) * fs->bytes_per_sector; // update file size if needed
    if (file_size_bytes < (int)required_size)
    {
        int data_size_bytes = 0;
        for (int i = fs->bytes_per_sector - 1; i >= 0; i--)
        { // data may not fill a sector
            if (data[i] != 0)
            {
                data_size_bytes = i + 1;
                break;
            }
        }

        file_size_bytes = sector_offset * fs->bytes_per_sector + data_size_bytes;

        int sector_offset = byte_offset / fs->bytes_per_sector;
        int sector_byte_offset = byte_offset % fs->bytes_per_sector;

        byte *buffer = fs->di->read(sector_offset);
        int_to_bytes(required_size, buffer, 28 + sector_byte_offset); // update file size on disk entry too
        fs->di->write(sector_offset, buffer);
        delete[] buffer;
    }

    return success;
}

/* Finds the sub directory with the matching given name, fails by returning null if no match is found */
FAT32::Entry *FAT32::Entry::find_sub_dir(string name)
{
    for (int i = 0; i < sub_dirs.size(); i++)
    {
        Entry *dir = sub_dirs[i];
        if (dir->name == name)
            return dir;
    }

    return nullptr; // couldn't find sub dir with matching name
}

/* Used for creating a new directory entry in memory as well as on disk with the given name under the given
parent directory. */
int FAT32::Entry::new_dir_entry(Entry *par_dir, string name, EntryAttribute type)
{
    this->parent_dir = par_dir;
    this->name = name;
    this->type = type;
    this->fs = par_dir->fs;
    this->order = -1;

    int name_size = name.size();
    int num_long_name_entries = (name_size / 13) + 1; // conpute the number of required long file name entries for the name

    if (name_size % 13 == 0)
        num_long_name_entries--; // edge case where the name will fill the last long file name entry - usually it will need an 'end' char but not in this case

    int first_entry_byte_offset = parent_dir->find_empty_entries(num_long_name_entries + 1); // find a continous region to store long name entries on disk

    if (first_entry_byte_offset == failure) // not enough space in data region for long name entries (at the very least not a continous space)
        return failure;

    this->byte_offset = first_entry_byte_offset + (num_long_name_entries * fs->bytes_per_dir_entry);

    if (!allocate_more_clusters(1)) // allocate a single cluster
        return failure;

    if (type == FILE)
        this->file_size_bytes = 0;
    else
        this->file_size_bytes = fs->bytes_per_cluster; // idk if directory entries actually need this field

    for (int i = 0; i < num_long_name_entries; i++)
    { // create long name entries in memory
        string name_part = "";
        int start_index = i * 13;
        int end_index = (i * 13) + 13;
        if (end_index >= (int)name.size())
            name_part = name.substr(start_index);
        else
            name_part = name.substr(start_index, end_index);

        Entry *long_name = new Entry();
        long_name->fs = fs;
        long_name->parent_dir = this;
        long_name->name = name_part;
        long_name->extension = "";
        long_name->type = LONG_NAME;
        long_name->byte_offset = first_entry_byte_offset + ((num_long_name_entries - i - 1) * fs->bytes_per_dir_entry);
        long_name->order = i + 1;
        long_name->last = false;

        long_name_entries.push_back(long_name);
    }

    long_name_entries.back()->last = true;
    write_dir_entry(); // write the new directory and all its long name entries to disk
    par_dir->sub_dirs.push_back(this);

    return success;
}
