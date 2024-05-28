#include "string.h"

extern "C" void *memcpy(void *dest, const void *src, size_t count)
{
    char *d = (char *)dest;
    const char *s = (const char *)src;
    for (int i = 0; i < count; i++)
    {
        d[i] = s[i];
    }
    return dest;
}

void memset(char *s, int size, int v)
{
    for (int i = 0; i < size; i++)
    {
        s[i] = (char)v;
    }
}

int strlen(const char *s)
{
    int c = 0;
    if (s == nullptr)
    {
        return 0;
    }
    while (s[c] != '\0')
    {
        c++;
    }
    return c;
}

void strcpy(char *dest, const char *src)
{
    if (dest == nullptr || src == nullptr)
    {
        return;
    }
    while (*src != '\0')
    {
        *dest = *src;
        dest++;
        src++;
    }
    *dest = '\0';
}
