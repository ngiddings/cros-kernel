#ifndef KERNEL_STATUS_H
#define KERNEL_STATUS_H

#ifdef __cplusplus
extern "C"
{
#endif

    enum error_t
    {
        ENONE = 0,
        EUNKNOWN = -1,
        ENOSYS = -2,
        EEOF = -3,
        ENOFILE = -4,
        ENOMEM = -5,
        EINVAL = -6,
        EIO = -7,
        EEXISTS = -8,
        EPIPE = -9,
        EFULL = -10
    };

#ifdef __cplusplus
}
#endif

#endif