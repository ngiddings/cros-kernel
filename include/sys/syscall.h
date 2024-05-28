#ifndef KERNEL_SYSCALL_H
#define KERNEL_SYSCALL_H

#include "types/syscallid.h"
#include "types/pid.h"

#ifdef __cplusplus
extern "C"
{
#endif

    int do_syscall(long id, unsigned long arg1, unsigned long arg2, unsigned long arg3, unsigned long arg4);

    /**
     * @brief Prints `str` on the kernel log.
     * @param str
     * @return 0
     */
    static inline int printk(const char *str)
    {
        return do_syscall(SYS_PRINTK, (unsigned long)str, 0, 0, 0);
    }

    /**
     * @brief Map a region in memory to newly allocated page frames
     * @param ptr Pointer to the start of the region to map
     * @param size Size in bytes of the region to map
     * @param flags Access flags for the pages to map
     * @return
     */
    static inline int mmap(void *ptr, unsigned long size, int flags)
    {
        return do_syscall(SYS_MMAP, (unsigned long)ptr, size, (unsigned long)flags, 0);
    }

    /**
     * @brief Free the memory in a particular region
     * @param ptr Pointer to the start of the region to unmap
     * @param size Size in bytes of the region to unmap
     * @return
     */
    static inline int munmap(void *ptr, unsigned long size)
    {
        return do_syscall(SYS_MUNMAP, (unsigned long)ptr, size, 0, 0);
    }

    /**
     * @brief Create a new process which shares the current process's address space,
     * but has a separate context and is scheduled separately.
     *
     * @param fn Function pointer to start executing the new process at
     * @param stack Stack pointer for the new process
     * @param flags
     * @return
     */
    static inline int clone(int (*fn)(void *), void *stack, void *userdata, int flags)
    {
        return do_syscall(SYS_CLONE, (unsigned long)fn, (unsigned long)stack, (unsigned long)userdata, (unsigned long)flags);
    }

    /**
     * @brief Completely terminate the current process, freeing all resources that
     * belong to it.
     *
     * @return
     */
    static inline int terminate()
    {
        return do_syscall(SYS_TERMINATE, 0, 0, 0, 0);
    }

    /**
     * @brief Replace the current process's address space with a fresh one, then
     * load a new program image from the executable specified by `path`.
     *
     * @param path Path to the executable to load
     * @param argv Program arguments to pass to the new program
     * @param envp Environment variables to pass to the new program
     * @return
     */
    static inline int exec(const char *path, char *const argv[], char *const envp[])
    {
        return do_syscall(SYS_EXEC, (unsigned long)path, (unsigned long)argv, (unsigned long)envp, 0);
    }

    /**
     * @brief Put current process at the end of the scheduler queue, then switch to
     * next process.
     *
     * @return
     */
    static inline int yield()
    {
        return do_syscall(SYS_YIELD, 0, 0, 0, 0);
    }

    /**
     * @brief Call the specified signal handler on the process with id `pid`.
     * @param pid Process to call a signal handler on
     * @param signal Signal handler to call
     * @return
     */
    static inline int sigraise(pid_t pid, int signal)
    {
        return do_syscall(SYS_SIGRAISE, (unsigned long)pid, (unsigned long)signal, 0, 0);
    }

    /**
     * @brief Return from a signal handler, putting the stack and process context
     * back to the state they were in just before the signal was triggered.
     * @return
     */
    static inline int sigret()
    {
        return do_syscall(SYS_SIGRET, 0, 0, 0, 0);
    }

    /**
     * @brief Stop scheduling process until a signal occurs.
     * @return
     */
    static inline int sigwait()
    {
        return do_syscall(SYS_SIGWAIT, 0, 0, 0, 0);
    }

    /**
     * @brief Sets the handler function to call when a particular signal occurs.
     * Kernel will pass the pointer `userdata` to the handler function when it is
     * called.
     * @param signal Signal to specify handler for
     * @param handler Function pointer to signal handler
     * @param trampoline Function pointer to trampoline function called when handler returns.
     * @param userdata Userdata to pass to handler function (can be NULL)
     * @return
     */
    static inline int sigaction(int signal, void (*handler)(void *), void (*trampoline)(void), void *userdata)
    {
        return do_syscall(SYS_SIGACTION, (unsigned long)signal, (unsigned long)handler, (unsigned long)trampoline, (unsigned long)userdata);
    }

    /**
     * @brief Open the file specified by `path`
     * @param path Path of the file to open
     * @param flags
     * @return The file descriptor for the file just opened
     */
    static inline int openf(const char *path, int flags)
    {
        return do_syscall(SYS_OPEN, (unsigned long)path, (unsigned long)flags, 0, 0);
    }

    /**
     * @brief Close a proviously open file
     * @param fd File descriptor of the open file to close
     * @return
     */
    static inline int closef(int fd)
    {
        return do_syscall(SYS_CLOSE, (unsigned long)fd, 0, 0, 0);
    }

    /**
     * @brief Create a new file at the given path
     * @param path Path of the file to create
     * @param flags Mode for the new file
     * @return
     */
    static inline int create(const char *path, int flags)
    {
        return do_syscall(SYS_CREATE, (unsigned long)path, (unsigned long)flags, 0, 0);
    }

    /**
     * @brief
     * @param fd
     * @return
     */
    static inline int unlink(int fd)
    {
        return do_syscall(SYS_UNLINK, (unsigned long)fd, 0, 0, 0);
    }

    /**
     * @brief
     * @param fd
     * @param buffer
     * @param size
     * @return
     */
    static inline int read(int fd, void *buffer, unsigned long size)
    {
        return do_syscall(SYS_READ, (unsigned long)fd, (unsigned long)buffer, size, 0);
    }

    /**
     * @brief
     * @param fd
     * @param buffer
     * @param size
     * @return
     */
    static inline int write(int fd, const void *buffer, unsigned long size)
    {
        return do_syscall(SYS_WRITE, (unsigned long)fd, (unsigned long)buffer, size, 0);
    }

    static inline int fddup(int oldfd, int newfd)
    {
        return do_syscall(SYS_FDDUP, (unsigned long)oldfd, (unsigned long)newfd, 0, 0);
    }

    static inline int create_pipe(int pipefd[2])
    {
        return do_syscall(SYS_CREATE_PIPE, (unsigned long)pipefd, 0, 0, 0);
    }

#ifdef __cplusplus
}
#endif

#endif