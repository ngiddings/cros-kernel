/*
 * Author: Hunter Overstake
 * Email: huntstake@gmail.com
 * Date: 5/14/2024
 */

#include "helpers.h"

/* Will convert the sequence of bytes in a given byte array starting at the given start index to an
unsigned short, will only read enough bytes required by an unsigned short. */
unsigned short bytes_to_short(const byte *bytes, int index)
{
    int size = sizeof(unsigned short);
    byte temp[size];
    for (int i = 0; i < size; i++)
        temp[i] = bytes[i + index];
    return *reinterpret_cast<unsigned short *>(temp);
}

/* Will convert the sequence of bytes in a given byte array starting at the given start index to an
unsigned int, will only read enough bytes required by an unsigned int. */
unsigned int bytes_to_int(const byte *bytes, int index)
{
    int size = sizeof(unsigned int);
    byte temp[size];
    for (int i = 0; i < size; i++)
        temp[i] = bytes[i + index];
    return *reinterpret_cast<unsigned int *>(temp);
}

/* Will convert the sequence of bytes in a given byte array starting at the given start index to an
unsigned long, will only read enough bytes required by an unsigned long. */
unsigned long bytes_to_long(const byte *bytes, int index)
{
    int size = sizeof(unsigned long);
    byte temp[size];
    for (int i = 0; i < size; i++)
        temp[i] = bytes[i + index];
    return *reinterpret_cast<unsigned long *>(temp);
}

/* Will convert the given unsigned short to a sequence of bytes in a given byte array starting at a
given index. Will only convert enough bytes required by an unsigned short. */
void short_to_bytes(unsigned short num, byte *buffer, int offset)
{
    int size = sizeof(unsigned short);
    for (int i = 0; i < size; i++)
        buffer[i + offset] = ((num >> (i * 8)) & 0xFF);
    return;
}

/* Will convert the given unsigned int to a sequence of bytes in a given byte array starting at a
given index. Will only convert enough bytes required by an unsigned int. */
void int_to_bytes(unsigned int num, byte *buffer, int offset)
{
    int size = sizeof(unsigned int);
    for (int i = 0; i < size; i++)
        buffer[i + offset] = ((num >> (i * 8)) & 0xFF);
    return;
}

/* Will convert the given unsigned long to a sequence of bytes in a given byte array starting at a
given index. Will only convert enough bytes required by an unsigned long. */
void long_to_bytes(unsigned long num, byte *buffer, int offset)
{
    int size = sizeof(unsigned long);
    for (int i = 0; i < size; i++)
        buffer[i + offset] = ((num >> (i * 8)) & 0xFF);
    return;
}

/* Given two characters, will return the unicode character of them slapped together. */
wchar_t chars_to_unicode(unsigned char first, unsigned char second)
{
    return (static_cast<wchar_t>(first) << 8) | static_cast<wchar_t>(second);
}
