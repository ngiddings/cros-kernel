#include "hasrefcount.h"

HasRefcount::HasRefcount()
    : refcount(0)
{
}

int HasRefcount::getRefCount() const
{
    return refcount;
}

void HasRefcount::addReference()
{
    refcount++;
}

void HasRefcount::removeReference()
{
    refcount--;
}