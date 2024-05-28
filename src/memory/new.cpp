#include "new.h"
#include "heap.h"

void *operator new(size_t size)
{
    return rmalloc(size);
}

void operator delete(void *ptr)
{
    rfree(ptr);
}

void *operator new[](size_t size)
{
    return rmalloc(size);
}

void operator delete[](void *ptr)
{
    return rfree(ptr);
}

void operator delete(void *ptr, unsigned long sz)
{
    return rfree(ptr);
}

void operator delete[](void *ptr, unsigned long sz)
{
    return rfree(ptr);
}