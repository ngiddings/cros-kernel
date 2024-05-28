#ifndef KERNEL_STRING_H
#define KERNEL_STRING_H

#include <cstddef>

/**
 * @brief
 * @see https://en.cppreference.com/w/c/string/byte/memcpy
 * @param dest
 * @param src
 * @param count
 * @return
 */
extern "C" void *memcpy(void *dest, const void *src, size_t count);

extern "C" void memset(char *s, int size, int v);

extern "C" int strlen(const char *s);

extern "C" void strcpy(char *dest, const char *src);

#endif