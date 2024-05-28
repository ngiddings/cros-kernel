/*
 * Author: Hunter Overstake
 * Email: huntstake@gmail.com
 * Date: 5/14/2024
 */

#include "util/log.h"
#include "containers/vector.h"
#include "containers/pair.h"
#include "fat32.h"

/* Finds the entry base on a given file path, fails if: the path is invalid, couldn't find entry via the path. */
FAT32::Entry *FAT32::find_entry(string file_path)
{
    if (file_path == "/")
        return root_dir;

    vector<string> path = parse_file_path(file_path);

    if (path.empty())
        return nullptr; // couldn't parse file_path

    Entry *current = root_dir;
    for (int i = 0; i < (int)path.size(); i++)
    {
        current = current->find_sub_dir(path[i]);
        if (current == nullptr)
            return nullptr; // couldn't find sub dir with matching name
    }

    return current;
}

/* Parses a given file path into its corresponding sequence of entry string names. */
vector<string> FAT32::parse_file_path(string file_path)
{
    vector<string> path;

    if (file_path[0] != '/')
        return path;

    string part = "";
    for (int i = 1; i < (int)file_path.size(); i++)
    {
        char c = file_path[i];
        if (c == '/')
        {
            path.push_back(part);
            part = "";
        }
        else
        {
            part += c;
        }
    }
    path.push_back(part);

    return path;
}

/* Cuts off the last entry name of the file path, used for getting the parent directory file path. */
string FAT32::snip_file_path(string file_path)
{
    int last_slash_index = 0;
    for (int i = 1; i < (int)file_path.size(); i++)
    {
        if (file_path[i] == '/')
            last_slash_index = i;
    }

    string snipped = "/";

    if (last_slash_index != 0)
        snipped = file_path.substr(0, last_slash_index);
    return snipped;
}

// below this are temp functions used for testing/debugging, remove when merged with kernel

void FAT32::dump_fs_structure()
{
    // Use a stack to perform iterative depth-first traversal
    vector<pair<Entry *, int>> s;
    s.push_back({root_dir, 0});

    while (!s.empty())
    {
        Entry *current = s.back().first;
        vector<Entry *> sub_dirs = current->sub_dirs;
        int num_sub_dirs = (int)sub_dirs.size();
        int depth = s.back().second;
        s.pop_back();

        // Print current directory name
        for (int i = 0; i < depth; ++i)
        {
            printf("|   ");
        }
        printf("|-- %s\n", current->name.c_str());

        // Push subdirectories onto the stack in reverse order
        for (int i = num_sub_dirs - 1; i >= 0; i--)
        {
            s.push_back({sub_dirs[i], depth + 1});
        }
    }
}

void FAT32::fat_dump(int cluster_offset)
{
    int sector_offset = reserved_sectors + (sectors_per_cluster * cluster_offset);

    printf("\n");

    for (int i = 0; i < sectors_per_cluster; i++)
    {
        byte *sector = di->read(i + sector_offset);

        for (int j = 0; j < bytes_per_sector; j += 4)
        {
            printf("%i ", bytes_to_int(sector, j));
        }
        delete[] sector;
    }
    printf("\n");
}

void FAT32::mem_dump(int cluster_offset)
{
    int sector_offset = (sectors_per_cluster * cluster_offset);

    printf("\n");

    for (int i = 0; i < sectors_per_cluster; i++)
    {
        byte *sector = di->read(i + sector_offset);

        for (int j = 0; j < bytes_per_sector * sectors_per_cluster; j++)
        {
            if (sector[j] == 0)
            {
                printf("_ ");
            }
            else
            {
                printf("%02x ", sector[j]);
            }
        }

        delete[] sector;
    }

    printf("\n");
}

int FAT32::test()
{
    byte *buffer;

    if (!read_file("/folder2/file6.txt", 0, buffer))
    {
        kernelLog(LogLevel::WARNING, "FAT32 Test: read failure");
        delete[] buffer;
        return failure;
    }

    vector<string> list;
    if (!list_dir("/folder2", list))
    {
        kernelLog(LogLevel::WARNING, "FAT32 Test: list failure");
        return failure;
    }

    if (!write_file("/file2.txt", 0, buffer))
    {
        kernelLog(LogLevel::WARNING, "FAT32 Test: write failure");
        return failure;
    }

    if (!create_file("/folder2/griddy_uncanny.ohi"))
    {
        kernelLog(LogLevel::WARNING, "FAT32 Test: file creation failure");
        return failure;
    }

    if (!write_file("/folder2/griddy_uncanny.ohi", 0, buffer))
    {
        kernelLog(LogLevel::WARNING, "FAT32 Test: write failure");
        return failure;
    }

    if (!create_dir("/folder1/skibidi_gyatt_rizzler"))
    {
        kernelLog(LogLevel::WARNING, "FAT32 Test: directory creation failure");
        return failure;
    }

    if (!create_file("/folder1/skibidi_gyatt_rizzler/yolo.txt"))
    {
        kernelLog(LogLevel::WARNING, "FAT32 Test: file creation failure");
        return failure;
    }

    if (!write_file("/folder1/skibidi_gyatt_rizzler/yolo.txt", 0, buffer))
    {
        kernelLog(LogLevel::WARNING, "FAT32 Test: write failure");
        return failure;
    }

    delete[] buffer;

    return success;
}