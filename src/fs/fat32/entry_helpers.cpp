/*
 * Author: Hunter Overstake
 * Email: huntstake@gmail.com
 * Date: 5/14/2024
 */

#include "fat32.h"

/* Parses this entry's attribute from disk to find out what type of entry this is. */
FAT32::EntryAttribute FAT32::Entry::parse_attribute(int byte_offset)
{
    int sector_offset = byte_offset / fs->bytes_per_sector;
    int sector_byte_offset = byte_offset % fs->bytes_per_sector;

    byte *buffer = fs->di->read(sector_offset);

    FAT32::EntryAttribute type = EMPTY;

    unsigned char attribute = buffer[11 + sector_byte_offset];
    attribute &= 0x3F; // upper 2 bits are reserved, so we must mask those 2 bits when reading the value
    switch (attribute)
    {
    case 0x08:
        type = ROOT;
        break;
    case 0x10:
        type = DIRECTORY;
        break;
    case 0x20:
        type = FILE;
        break;
    case 0x0F:
        type = LONG_NAME;
        break;
    default:
        type = EMPTY;
        break;
    }

    delete[] buffer;
    return type;
}

/* Parses the short file name of this entry from disk. */
string FAT32::Entry::parse_short_file_name(int byte_offset)
{
    int sector_offset = byte_offset / fs->bytes_per_sector;
    int sector_byte_offset = byte_offset % fs->bytes_per_sector;

    byte *buffer = fs->di->read(sector_offset);

    string name = "";
    for (int i = 0; i < 8; i++)
    {
        char c = buffer[i + sector_byte_offset];
        if (c != ' ')
            name += c;
    }

    delete[] buffer;

    return name;
}

/* Treats this entry as the root directory while parsing its meta-data from disk. */
void FAT32::Entry::parse_root_entry()
{
    int sector_offset = byte_offset / fs->bytes_per_sector;
    int sector_byte_offset = byte_offset % fs->bytes_per_sector;

    byte *buffer = fs->di->read(sector_offset);

    name = "";
    extension = "";

    find_cluster_allocations((unsigned int)fs->reserved_fat_entries); // just used as an offset, dont read into it too much

    file_size_bytes = bytes_to_int(buffer, 28 + sector_byte_offset);

    delete[] buffer;

    order = -1;

    find_sub_dir_entries(); // start of the recursive search of the file system
}

/* Treats this entry as directory while parsing its meta-data from disk - also used
for entries of type file since their actual disk entry structure is identical. */
void FAT32::Entry::parse_dir_entry()
{
    int sector_offset = byte_offset / fs->bytes_per_sector;
    int sector_byte_offset = byte_offset % fs->bytes_per_sector;

    byte *buffer = fs->di->read(sector_offset);

    name = parse_short_file_name(byte_offset);

    extension = "";
    for (int i = 8; i < 11; i++)
    {
        char c = buffer[i + sector_byte_offset];
        if (c != ' ')
            extension += c;
    }

    unsigned short first_cluster_high = bytes_to_short(buffer, 20 + sector_byte_offset); // first cluster num is split up for some reason
    unsigned short first_cluster_low = bytes_to_short(buffer, 26 + sector_byte_offset);
    unsigned int first_cluster_offset = (static_cast<unsigned int>(first_cluster_high) << 16) | first_cluster_low;

    find_cluster_allocations(first_cluster_offset);

    file_size_bytes = bytes_to_int(buffer, 28 + sector_byte_offset);

    delete[] buffer;

    find_long_name_entries(); // also updates this entries file name if it finds any long file name entries

    order = -1;

    if (type == DIRECTORY)
        find_sub_dir_entries(); // keep searching
}

/* Treats this entry as a long file name while parsing its meta-data from disk. */
void FAT32::Entry::parse_long_name()
{
    int sector_offset = byte_offset / fs->bytes_per_sector;
    int sector_byte_offset = byte_offset % fs->bytes_per_sector;

    byte *buffer = fs->di->read(sector_offset);

    order = buffer[0 + sector_byte_offset] & 0x1F;
    last = false;
    name = "";

    // name pieces are split up for some reason
    int offset = 1;
    for (int i = 0; i < 10; i += 2)
    {
        byte first = buffer[i + offset + sector_byte_offset];
        byte second = buffer[i + 1 + offset + sector_byte_offset];
        wchar_t unicode = chars_to_unicode(first, second);
        if (unicode != L'\xFFFF')
            name += first;
        else
            break;
    }

    offset = 14;
    for (int i = 0; i < 12; i += 2)
    {
        byte first = buffer[i + offset + sector_byte_offset];
        byte second = buffer[i + 1 + offset + sector_byte_offset];
        wchar_t unicode = chars_to_unicode(first, second);
        if (unicode != L'\xFFFF')
            name += first;
        else
            break;
    }

    offset = 28;
    for (int i = 0; i < 4; i += 2)
    {
        byte first = buffer[i + offset + sector_byte_offset];
        byte second = buffer[i + 1 + offset + sector_byte_offset];
        wchar_t unicode = chars_to_unicode(first, second);
        if (unicode != L'\xFFFF')
            name += first;
        else
            break;
    }

    extension = "";
    file_size_bytes = -1;

    delete[] buffer;
}

/* Finds all the clusters allocated to this entry given its first cluster offset in the FAT */
void FAT32::Entry::find_cluster_allocations(unsigned int first_fat_entry_offset)
{
    if (first_fat_entry_offset == 0)
        return;

    unsigned int current_fat_entry_offset = first_fat_entry_offset;
    int current_fat_sector_offset = fs->reserved_sectors + (first_fat_entry_offset * fs->bytes_per_fat_entry) / fs->bytes_per_sector;
    int current_fat_secotor_int_offset = (current_fat_entry_offset * fs->bytes_per_fat_entry) % fs->bytes_per_sector;

    byte *buffer = fs->di->read(current_fat_sector_offset);

    FatEntryType type = check_fat_entry_type(bytes_to_int(buffer, current_fat_secotor_int_offset));

    while (type == ALLOCATED)
    {
        fat_allocations.push_back(current_fat_entry_offset);

        unsigned int next_fat_entry_offset = bytes_to_int(buffer, current_fat_secotor_int_offset);

        int next_fat_sector_offset = fs->reserved_sectors + (next_fat_entry_offset * fs->bytes_per_fat_entry) / fs->bytes_per_sector;
        if (check_fat_entry_type(next_fat_entry_offset) == ALLOCATED && next_fat_sector_offset != current_fat_sector_offset)
        { // update the sector being read from if the next cluster is outside the current sector
            delete[] buffer;
            buffer = fs->di->read(next_fat_sector_offset);
        }

        current_fat_entry_offset = next_fat_entry_offset;
        current_fat_sector_offset = next_fat_sector_offset;
        current_fat_secotor_int_offset = (current_fat_entry_offset * fs->bytes_per_fat_entry) % fs->bytes_per_sector;

        type = check_fat_entry_type(current_fat_entry_offset);
    }
    fat_allocations.push_back(current_fat_entry_offset);

    delete[] buffer;
}

/* Given the raw value of the fat cluster entry, returns the type - see offical docs for explaintion on the values. */
FAT32::FatEntryType FAT32::Entry::check_fat_entry_type(unsigned int val)
{
    unsigned int snipped_val = val & 0xFFFFFFF;
    if (val == 0xFFFFFFFF)
        return END;
    switch (snipped_val)
    {
    case 0x0000000:
        return FREE;
    case 0xFFFFFFE:
        return DEFECTIVE;
    default:
        unsigned int total_num_clusters = fs->total_sectors / fs->sectors_per_cluster;
        if (val < total_num_clusters)
            return ALLOCATED;
        else
            return RESERVED;
    }
}

/* Finds the long file name entries associated with this entry, doesn't search beyond the current cluster, needs
refactoring if you want to support file/dir names longer than 208 characters long. */
void FAT32::Entry::find_long_name_entries()
{
    int current_cluster_start_byte_offset = byte_offset / fs->bytes_per_cluster;
    int search_byte_offset = byte_offset - fs->bytes_per_dir_entry;

    while (search_byte_offset >= current_cluster_start_byte_offset)
    { // only search within current cluster
        if (parse_attribute(search_byte_offset) == LONG_NAME)
        {
            Entry *long_name = new Entry(fs, this, search_byte_offset);
            long_name_entries.push_back(long_name);
        }
        else
        {
            break;
        }
        search_byte_offset -= fs->bytes_per_dir_entry;
    }

    if (!long_name_entries.empty())
    {
        name = "";
        for (int i = 0; i < (int)long_name_entries.size(); i++)
        { // construct the full name of this entry from the long file names
            name += long_name_entries[i]->name;
        }
        long_name_entries.back()->last = true; // mark the last one
    }

    return;
}

/* Searches through all of this entries clusters and finds all of its sub-directoies and creates memory entries for each, passes over long
file names as they are owned by sub-directors - only call this function once per entry. */
void FAT32::Entry::find_sub_dir_entries()
{
    for (int i = 0; i < (int)fat_allocations.size(); i++)
    {
        int search_cluster_offset = fat_entry_to_cluster_offset(fat_allocations[i]);
        int serach_cluster_start_byte_offset = search_cluster_offset * fs->bytes_per_cluster;
        int serach_cluster_end_byte_offset = serach_cluster_start_byte_offset + fs->bytes_per_cluster;
        int search_byte_offset = serach_cluster_start_byte_offset;

        while (search_byte_offset < serach_cluster_end_byte_offset)
        {
            EntryAttribute type = parse_attribute(search_byte_offset);
            string name = parse_short_file_name(search_byte_offset);

            bool is_dir_or_file = (type == DIRECTORY || type == FILE);
            bool is_dot_or_dotdot = (name == "." || name == "..");

            if (is_dir_or_file && !is_dot_or_dotdot)
            { // make sure they aren't the two required copy-entries
                Entry *sub_dir = new Entry(fs, this, search_byte_offset);
                sub_dirs.push_back(sub_dir);
            }

            search_byte_offset += fs->bytes_per_dir_entry;
        }
    }
}

/* Converts the fat entry value to its corresponding cluster offset in the data region. */
int FAT32::Entry::fat_entry_to_cluster_offset(unsigned int fat_entry)
{
    return (fat_entry - fs->reserved_fat_entries) + fs->data_region_start_cluster_offset;
}

/* Allocats a given number of new clusters to this entry, will fail if: there are no remaining
free clusters in the data region. */
int FAT32::Entry::allocate_more_clusters(int num_additional_clusters)
{
    if (num_additional_clusters <= 0)
        return failure; // yeah just don't do that

    int fat_start_byte_offset = fs->reserved_sectors * fs->bytes_per_sector;

    if (fat_allocations.empty())
    { // allocate first cluster if none are allocated yet
        if (!initial_cluster())
            return failure;
        num_additional_clusters--;
    }

    int current_entry_offset = fat_allocations[(int)fat_allocations.size() - 1];
    int current_entry_byte_offset = fat_start_byte_offset + current_entry_offset * fs->bytes_per_fat_entry;
    int current_entry_sector_offset = current_entry_byte_offset / fs->bytes_per_sector;
    int current_entry_sector_byte_offset = current_entry_byte_offset % fs->bytes_per_sector;

    byte *buffer = fs->di->read(current_entry_sector_offset);

    bool no_more_clusters = false;
    for (int i = 0; i < num_additional_clusters; i++)
    {
        int next_free_offset = find_free_cluster();

        if (next_free_offset == failure)
        {
            no_more_clusters = true;
            break;
        }

        int next_entry_offset = next_free_offset;
        int next_entry_byte_offset = fat_start_byte_offset + next_entry_offset * fs->bytes_per_fat_entry;
        int next_entry_sector_offset = next_entry_byte_offset / fs->bytes_per_sector;
        int next_entry_sector_byte_offset = next_entry_byte_offset % fs->bytes_per_sector;

        int_to_bytes(next_entry_offset, buffer, current_entry_sector_byte_offset);
        fs->di->write(current_entry_sector_offset, buffer);
        delete[] buffer;
        buffer = fs->di->read(next_entry_sector_offset);

        current_entry_offset = next_entry_offset;
        current_entry_byte_offset = next_entry_byte_offset;
        current_entry_sector_offset = next_entry_sector_offset;
        current_entry_sector_byte_offset = next_entry_sector_byte_offset;

        if (type == DIRECTORY)
            init_dir_cluster(fat_entry_to_cluster_offset(current_entry_offset)); // if this entry is a directory, each cluster allocated to it needs some setup done to it, see init_dir_cluster() for details

        fat_allocations.push_back(current_entry_offset);
    }

    unsigned int end_marker = 0xFFFFFFFF; //  mark last fat entry as the end
    int_to_bytes(end_marker, buffer, current_entry_sector_byte_offset);
    fs->di->write(current_entry_sector_offset, buffer);

    delete[] buffer;

    if (no_more_clusters)
        return failure; // no more space in data region
    return success;
}

int FAT32::Entry::find_free_cluster()
{
    int fat_start_sector_offset = fs->reserved_sectors;
    int fat_start_byte_offset = fat_start_sector_offset * fs->bytes_per_sector;
    int clusters_per_fat = fs->sectors_per_fat / fs->sectors_per_cluster;

    int current_entry_offset = 2;
    int current_entry_byte_offset = fat_start_byte_offset + current_entry_offset * fs->bytes_per_fat_entry;
    int current_entry_sector_offset = current_entry_byte_offset / fs->bytes_per_sector;
    int current_entry_sector_byte_offset = current_entry_byte_offset % fs->bytes_per_sector;

    byte *buffer = fs->di->read(current_entry_sector_offset);

    while (current_entry_offset < clusters_per_fat)
    {
        unsigned int entry_val = bytes_to_int(buffer, current_entry_sector_byte_offset);
        FatEntryType type = check_fat_entry_type(entry_val);
        if (type == FREE)
        {
            delete[] buffer;
            return current_entry_offset;
        }

        int next_entry_offset = current_entry_offset + 1;
        int next_entry_byte_offset = fat_start_byte_offset + next_entry_offset * fs->bytes_per_fat_entry;
        int next_entry_sector_offset = next_entry_byte_offset / fs->bytes_per_sector;
        int next_entry_sector_byte_offset = next_entry_byte_offset % fs->bytes_per_sector;

        if (next_entry_sector_offset != current_entry_sector_offset)
        {
            delete[] buffer;
            buffer = fs->di->read(next_entry_sector_offset);
        }

        current_entry_offset = next_entry_offset;
        current_entry_byte_offset = next_entry_byte_offset;
        current_entry_sector_offset = next_entry_sector_offset;
        current_entry_sector_byte_offset = next_entry_sector_byte_offset;
    }

    delete[] buffer;
    return failure;
}

/* Finds a continuous space big enough for the given number of entries within the clusters
allocated to this entry, will allocate more if it can't find a space, will fail if: it needs
to allocate more clusters but there is no more free clusters in the data region. */
int FAT32::Entry::find_empty_entries(int num_entries)
{
    int current_byte_offset;
    if (parent_dir == nullptr) // root dir
        current_byte_offset = parent_dir->byte_offset;
    else
        current_byte_offset = fat_entry_to_cluster_offset(fat_allocations[0]) * fs->bytes_per_cluster;

    int current_cluster_offset = byte_offset / fs->bytes_per_cluster;

    int num_empty_found = 0;
    int empty_start_offset = 0;
    int cluster_num = 1;
    int the_sky_is_blue = true;
    while (the_sky_is_blue)
    {
        EntryAttribute type = parse_attribute(current_byte_offset);
        if (type == EMPTY)
        {
            if (num_empty_found == 0)
                empty_start_offset = current_byte_offset;

            num_empty_found++;

            if (num_empty_found == num_entries)
                break;
        }
        else
        {
            num_empty_found = 0;
        }

        int next_byte_offset = current_byte_offset += fs->bytes_per_dir_entry;
        int next_cluster_offset = byte_offset / fs->bytes_per_cluster;

        if (next_cluster_offset != current_cluster_offset)
        {
            cluster_num++;
            if (cluster_num > (int)parent_dir->fat_allocations.size())
            { // can't find a space in the current cluster and this is the last cluster, so allocate another one
                if (!parent_dir->allocate_more_clusters(1))
                    return failure; // out of space in data region
            }

            next_byte_offset = fat_entry_to_cluster_offset(parent_dir->fat_allocations.back()) + (2 * fs->bytes_per_dir_entry); // skip first two entries, we know they aren't empty
            next_cluster_offset = byte_offset / fs->bytes_per_cluster;
        }

        current_byte_offset = next_byte_offset;
        current_cluster_offset = next_cluster_offset;
    }

    return empty_start_offset;
}

/* Intializes a given cluster of this directory entry with two base entries - '.' which
is a copy of this entry, and '..' which is a copy of this entry's parent entry. */
void FAT32::Entry::init_dir_cluster(int cluster_offset)
{
    int cluster_byte_offset = cluster_offset * fs->bytes_per_cluster;
    int sector_offset = byte_offset / fs->bytes_per_sector;
    int sector_byte_offset = byte_offset % fs->bytes_per_sector;

    clear_entry(cluster_byte_offset);
    clear_entry(cluster_byte_offset + fs->bytes_per_dir_entry);

    byte *buffer = fs->di->read(sector_offset);

    buffer[0 + sector_byte_offset] = '.';
    buffer[11 + sector_byte_offset] = 0x10;

    unsigned short first_cluster_high = (fat_allocations[0] >> 16) & 0xFFFF;
    unsigned short first_cluster_low = fat_allocations[0] & 0xFFFF;

    short_to_bytes(first_cluster_high, buffer, 20 + sector_byte_offset);
    short_to_bytes(first_cluster_low, buffer, 26 + sector_byte_offset);
    int_to_bytes(file_size_bytes, buffer, 28 + sector_byte_offset);

    sector_byte_offset += fs->bytes_per_dir_entry;

    buffer[0 + sector_byte_offset] = '.';
    buffer[0 + sector_byte_offset] = '.';
    buffer[11 + sector_byte_offset] = 0x10;

    first_cluster_high = (parent_dir->fat_allocations[0] >> 16) & 0xFFFF;
    first_cluster_low = parent_dir->fat_allocations[0] & 0xFFFF;

    short_to_bytes(first_cluster_high, buffer, 20 + sector_byte_offset);
    short_to_bytes(first_cluster_low, buffer, 26 + sector_byte_offset);
    int_to_bytes(parent_dir->file_size_bytes, buffer, 28 + sector_byte_offset);

    fs->di->write(sector_offset, buffer);

    delete[] buffer;
}

/* Clears the entry specified by the given byte offset, removes any garbage data that may be there. */
void FAT32::Entry::clear_entry(int byte_offset)
{
    int sector_offset = byte_offset / fs->bytes_per_sector;
    int sector_byte_offset = byte_offset % fs->bytes_per_sector;

    byte *buffer = fs->di->read(sector_offset);

    for (int i = 0; i < fs->bytes_per_dir_entry; i++)
        buffer[i + sector_byte_offset] = 0x0;

    fs->di->write(sector_offset, buffer);

    delete[] buffer;
}

/* Allocates the first cluster of this entry, assumes that there are no clusters already allocated to this
entry, will fail if: there are no free clusters left in the data region. */
int FAT32::Entry::initial_cluster()
{
    int fat_start_byte_offset = fs->reserved_sectors * fs->bytes_per_sector;

    int current_entry_offset = find_free_cluster();

    if (current_entry_offset == failure) // no free clusters left
        return failure;

    int current_entry_byte_offset = fat_start_byte_offset + current_entry_offset * fs->bytes_per_fat_entry;
    int current_entry_sector_offset = current_entry_byte_offset / fs->bytes_per_sector;
    int current_entry_sector_byte_offset = current_entry_byte_offset % fs->bytes_per_sector;

    byte *buffer = fs->di->read(current_entry_sector_offset);

    unsigned int end_marker = 0xFFFFFFFF;
    int_to_bytes(end_marker, buffer, current_entry_sector_byte_offset);
    fs->di->write(current_entry_sector_offset, buffer);
    delete[] buffer;

    fat_allocations.push_back(current_entry_offset);

    if (type == DIRECTORY)
        init_dir_cluster(fat_entry_to_cluster_offset(current_entry_offset)); // if directory, perform cluster setup

    return success;
}

/* Writes the meta-data stored in this file/directory memory entry to the corresponding entry on disk. */
void FAT32::Entry::write_dir_entry()
{
    clear_entry(byte_offset); // clear old data

    int sector_offset = byte_offset / fs->bytes_per_sector;
    int sector_byte_offset = byte_offset % fs->bytes_per_sector;

    byte *buffer = fs->di->read(sector_offset);

    string short_name = long_to_short_name(name, 8);
    for (int i = 0; i < 8; i++)
    {
        buffer[i + sector_byte_offset] = short_name[i];
    }

    string short_ext = long_to_short_name(extension, 3);
    for (int i = 0; i < 3; i++)
    {
        buffer[8 + i + sector_byte_offset] = short_ext[i];
    }

    if (type == FILE)
        buffer[sector_byte_offset + 11] = 0x20;
    else
        buffer[sector_byte_offset + 11] = 0x10;

    unsigned int first_cluster = fat_allocations[0];
    unsigned short cluster_high = (first_cluster >> 16) & 0xFFFF;
    unsigned short cluster_low = first_cluster & 0xFFFF;

    short_to_bytes(cluster_high, buffer, 20 + sector_byte_offset);
    short_to_bytes(cluster_low, buffer, 26 + sector_byte_offset);

    int_to_bytes(file_size_bytes, buffer, 28 + sector_byte_offset);

    fs->di->write(sector_offset, buffer);

    delete[] buffer;

    for (int i = 0; i < long_name_entries.size(); i++)
        long_name_entries[i]->write_long_name_entry(); // also update long file name disk entries
}

/* Writes the meta-data stored in this long file name memory entry to the corresponding entry on disk,
this function probably doesn't calcuate the checksum properly and it probably the reason why it isn't
comforming to the FAT32 standard fully. */
void FAT32::Entry::write_long_name_entry()
{
    clear_entry(byte_offset); // clear old data

    int sector_offset = byte_offset / fs->bytes_per_sector;
    int sector_byte_offset = byte_offset % fs->bytes_per_sector;

    byte *buffer = fs->di->read(sector_offset);

    if (last)
        buffer[sector_byte_offset] = (unsigned char)order | 0x40;
    else
        buffer[sector_byte_offset] = (unsigned char)order;

    bool done = false;
    string name_part_1 = "";
    int start_index = 0;
    int end_index = 5;
    if (end_index >= (int)name.size())
    {
        name_part_1 = name.substr(start_index);
        done = true;
    }
    else
    {
        name_part_1 = name.substr(start_index, end_index);
    }

    string name_part_2 = "";
    start_index = 5;
    end_index = 11;
    if (!done)
    {
        if (end_index >= (int)name.size())
        {
            name_part_2 = name.substr(start_index);
            done = true;
        }
        else
        {
            name_part_2 = name.substr(start_index, end_index);
        }
    }

    string name_part_3 = "";
    start_index = 11;
    end_index = 13;
    if (!done)
    {
        if (end_index >= (int)name.size())
        {
            name_part_3 = name.substr(start_index);
        }
        else
        {
            name_part_3 = name.substr(start_index, end_index);
        }
    }

    for (int i = 0; i < 5; i++)
    {
        if (i == (int)name_part_1.size())
        {
            buffer[sector_byte_offset + 1 + i * 2] = 0x0;
        }
        else if (i > name_part_1.size())
        {
            buffer[sector_byte_offset + 1 + i * 2] = 0xFF;
            buffer[sector_byte_offset + 1 + i * 2 + 1] = 0xFF;
        }
        else
        {
            buffer[sector_byte_offset + 1 + i * 2] = name_part_1[i];
        }
    }

    for (int i = 0; i < 6; i++)
    {
        if (i == (int)name_part_2.size())
        {
            buffer[sector_byte_offset + 14 + i * 2] = 0x0;
        }
        else if (i > name_part_2.size())
        {
            buffer[sector_byte_offset + 14 + i * 2] = 0xFF;
            buffer[sector_byte_offset + 14 + i * 2 + 1] = 0xFF;
        }
        else
        {
            buffer[sector_byte_offset + 14 + i * 2] = name_part_2[i];
        }
    }

    for (int i = 0; i < 2; i++)
    {
        if (i == name_part_3.size())
        {
            buffer[sector_byte_offset + 28 + i * 2] = 0x0;
        }
        else if (i > name_part_3.size())
        {
            buffer[sector_byte_offset + 28 + i * 2] = 0xFF;
            buffer[sector_byte_offset + 28 + i * 2 + 1] = 0xFF;
        }
        else
        {
            buffer[sector_byte_offset + 28 + i * 2] = name_part_3[i];
        }
    }

    buffer[sector_byte_offset + 11] = 0x0F;

    unsigned char checksum = 0;

    string short_name = long_to_short_name(parent_dir->name, 8);
    short_name += long_to_short_name(extension, 3);
    for (int i = 0; i < short_name.size(); i++)
    {
        char c = short_name[i];

        checksum = ((checksum & 1) ? 0x80 : 0) + (checksum >> 1) + c; // calculate checksum
        // TODO: this probably isnt correct
    }

    buffer[sector_byte_offset + 13] = checksum;

    fs->di->write(sector_offset, buffer);

    delete[] buffer;
}

/* Converts the given long file name to the short name format of a given length used by FAT32 file/directory disk entries. */
string FAT32::Entry::long_to_short_name(string long_name, int len)
{
    string short_name = "";
    for (int i = 0; i < len; i++)
    {
        char c = long_name[i];
        if (c >= 'A' && c <= 'Z')
        {
            short_name += c;
        }
        else if (c >= 'a' && c <= 'z')
        {
            c -= 'a' - 'A';
            short_name += c;
        }
        else if (
            c == '$' || c == '%' || c == '\'' || c == '-' || c == '_' ||
            c == '@' || c == '~' || c == '`' || c == '!' || c == '(' ||
            c == ')' || c == '{' || c == '}' || c == '^' || c == '#' ||
            c == '&' || c == '.')
        {
            short_name += c;
        }
    }

    for (int i = short_name.size(); i < len; i++)
    {
        short_name += ' ';
    }

    return short_name;
}
