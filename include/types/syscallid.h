#ifndef KERNEL_SYSCALLID_H
#define KERNEL_SYSCALLID_H

#ifdef __cplusplus
extern "C"
{
#endif

    typedef enum syscallid_t
    {
        SYS_PRINTK = 0,
        SYS_MMAP,
        SYS_MUNMAP,
        SYS_CLONE,
        SYS_TERMINATE,
        SYS_EXEC,
        SYS_YIELD,
        SYS_SIGRAISE,
        SYS_SIGRET,
        SYS_SIGWAIT,
        SYS_SIGACTION,
        SYS_OPEN,
        SYS_CLOSE,
        SYS_CREATE,
        SYS_UNLINK,
        SYS_READ,
        SYS_WRITE,
        SYS_FDDUP,
        SYS_CREATE_PIPE
    } syscallid_t;

#ifdef __cplusplus
}
#endif

#endif