#ifndef _KERNEL_HASREFCOUNT_H
#define _KERNEL_HASREFCOUNT_H

class HasRefcount
{
public:
    HasRefcount();

    virtual int getRefCount() const;

    virtual void addReference();

    virtual void removeReference();

protected:
    int refcount;
};

#endif