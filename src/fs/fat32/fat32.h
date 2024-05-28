#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H

/*
 * Author: Hunter Overstake
 * Email: huntstake@gmail.com
 * Date: 5/15/2024
 */

#include "disk_interface/disk_interface.h"
#include "helpers.h"

#define success 1
#define failure 0

enum FileType
{
    File = 0,
    Directory = 2,
};

/* A FAT32 file system interface which constructs a tree of the file system strucuture storing all the
 * neccesary meta-data and is used to read/write disk entries/data.  */
class FAT32
{
public:
    FAT32() = default;
    FAT32(string disk_filepath);
    FAT32(void *ramfs);
    ~FAT32();

    void init(byte *root_sector);

    int file_type(string file_path, FileType &type);
    int file_size(string file_path, int &size);
    int read_file(string file_path, int sector_offset, byte *&buffer);
    int write_file(string file_path, int sector_offset, const byte *data);
    int create_file(string file_path);
    int remove_file(string file_path);
    int copy_file(string file_path, string dest_dir);
    int move_file(string file_path, string dest_dir);

    int list_dir(string dir_path, vector<string> &list);
    int create_dir(string dir_path);
    int remove_dir(string dir_path);
    int copy_dir(string src_dir_path, string dest_dir_path);
    int move_dir(string src_dir_path, string dest_dir_path);

    int get_sector_size();

    // temp functions for testing/debugging, remove when merged with kernel
    void dump_fs_structure();
    void fat_dump(int cluster_offset);
    void mem_dump(int cluster_offset);
    int test();

private:
    enum EntryAttribute
    {
        FILE,
        EMPTY,
        DIRECTORY,
        LONG_NAME,
        ROOT
    };

    enum FatEntryType
    {
        END,
        FREE,
        RESERVED,
        ALLOCATED,
        DEFECTIVE
    };

    /* Memory representation of a disk entry, only storing the required/useful meta-data */
    class Entry
    {
    public:
        Entry() = default;
        Entry(FAT32 *fs, Entry *parent_dir, int byte_offset);

        ~Entry();

        string name;
        string extension; // may not have one
        EntryAttribute type;

        int file_size_bytes; // idk if directories or long file name entries really need this but I keep it updated anyway
        int byte_offset;     // actual byte offset of the corresponding disk entry
        int order;           // only used for entries of type LONG_NAME
        bool last;           // only used for entries of type LONG_NAME

        Entry *parent_dir;        // when entry type is LONG_NAME, this is used to link back to its associated FILE or DIRECTORY entry
        vector<Entry *> sub_dirs; // only for entries of type DIRECTORY
        vector<Entry *> long_name_entries;
        vector<int> fat_allocations;

        byte *read_data(int sector_offset);
        int write_data(int sector_offset, const byte *data);

        int new_dir_entry(Entry *par_dir, string name, EntryAttribute type);

        Entry *find_sub_dir(string name);

    private:
        FAT32 *fs;

        // these functions use explicit parameters and return types because they are used to inspect an entry without creating an entry object

        EntryAttribute parse_attribute(int byte_offset);
        string parse_short_file_name(int byte_offset);

        void parse_root_entry();
        void parse_dir_entry();
        void parse_long_name();
        void clear_entry(int byte_offset);

        void write_dir_entry();
        void write_long_name_entry();

        int allocate_more_clusters(int num_additional_clusters);
        int find_free_cluster();
        int initial_cluster();

        int find_empty_entries(int num_entries);
        void init_dir_cluster(int cluster_offset);

        void find_cluster_allocations(unsigned int first_fat_cluster_offset);
        void find_long_name_entries();
        void find_sub_dir_entries(); // use only once per entry

        FatEntryType check_fat_entry_type(unsigned int cluster_val);
        int fat_entry_to_cluster_offset(unsigned int fat_entry_val);
        string long_to_short_name(string long_name, int len);
    };

    DiskInterface *di;
    Entry *root_dir;
    int data_region_start_cluster_offset;

    int bytes_per_fat_entry;
    int bytes_per_dir_entry;
    int bytes_per_cluster;
    int bytes_per_sector;
    int sectors_per_cluster;
    int sectors_per_fat;
    int reserved_sectors;
    int reserved_fat_entries;
    int total_sectors;
    int fat_count;

    Entry *find_entry(string file_path);
    vector<string> parse_file_path(string file_path);
    string snip_file_path(string file_path);
};

#endif