#ifndef KERNEL_H
#define KERNEL_H

#include "memory/memorymap.h"
#include "fs/fat32/fat32.h"
#include "sched/queue.h"
#include "containers/binary_search_tree.h"
#include "types/pid.h"

/**
 * @brief Symbol located at the beginning of the kernel binary in memory.
 */
extern char __begin;

/**
 * @brief Symbol located at the end of the kernel binary in memory.
 */
extern char __end;

/**
 * @brief Symbol located at the start of high memory.
 */
extern char __high_mem;

extern "C" void (*syscall_table[])(long, long, long, long);

extern "C" kernel::sched::Context *do_syscall(unsigned long id, unsigned long arg1, unsigned long arg2, unsigned long arg3, unsigned long arg4, kernel::sched::Context *ctx);

namespace kernel
{

    void initialize(memory::MemoryMap &memoryMap, unsigned long kernelSize);

    class Kernel
    {
    public:
        Kernel();

        pid_t nextPid();

        int initMemory(memory::MemoryMap &memoryMap, unsigned long kernelSize);

        int initRamFS(void *ramfs);

        FAT32 *getRamFS();

        void switchTask();

        void setCallerReturn(unsigned long v);

        void addProcess(sched::Process &p);

        sched::Process *getActiveProcess();

        void sleepActiveProcess();

        void deleteActiveProcess();

        int raiseSignal(pid_t pid, int signal);

        int exec(const char *path, char *const argv[], char *const envp[]);

        void loadInitProgram();

    private:
        queue scheduler;

        binary_search_tree<pid_t, sched::Process> processTable;

        FAT32 *fs;

        pid_t currPid;
    };

    extern Kernel kernel;

    /**
     * @brief Prints `str` on the kernel log.
     * @param str
     * @return 0
     */
    void syscall_printk(const char *str);

    /**
     * @brief Map a region in memory to newly allocated page frames
     * @param ptr Pointer to the start of the region to map
     * @param size Size in bytes of the region to map
     * @param flags Access flags for the pages to map
     * @return
     */
    void syscall_mmap(void *ptr, unsigned long size, int flags);

    /**
     * @brief Free the memory in a particular region
     * @param ptr Pointer to the start of the region to unmap
     * @param size Size in bytes of the region to unmap
     * @return
     */
    void syscall_munmap(void *ptr, unsigned long size);

    /**
     * @brief Create a new process which shares the current process's address space,
     * but has a separate context and is scheduled separately.
     *
     * @param fn Function pointer to start executing the new process at
     * @param stack Stack pointer for the new process
     * @param flags
     * @return
     */
    void syscall_clone(int (*fn)(void *), void *stack, void *userdata, int flags);

    /**
     * @brief Completely terminate the current process, freeing all resources that
     * belong to it.
     *
     * @return
     */
    void syscall_terminate();

    /**
     * @brief Replace the current process's address space with a fresh one, then
     * load a new program image from the executable specified by `path`.
     *
     * @param path Path to the executable to load
     * @param argv Program arguments to pass to the new program
     * @param envp Environment variables to pass to the new program
     * @return
     */
    void syscall_exec(const char *path, char *const argv[], char *const envp[]);

    /**
     * @brief Put current process at the end of the scheduler queue, then switch to
     * next process.
     *
     * @return
     */
    void syscall_yield();

    /**
     * @brief Call the specified signal handler on the process with id `pid`.
     * @param pid Process to call a signal handler on
     * @param signal Signal handler to call
     * @return
     */
    void syscall_sigraise(pid_t pid, int signal);

    /**
     * @brief Return from a signal handler, putting the stack and process context
     * back to the state they were in just before the signal was triggered.
     * @return
     */
    void syscall_sigret();

    /**
     * @brief Stop scheduling process until a signal occurs.
     * @return
     */
    void syscall_sigwait();

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
    void syscall_sigaction(int signal, void (*handler)(void *), void (*trampoline)(void), void *userdata);

    /**
     * @brief Open the file specified by `path`
     * @param path Path of the file to open
     * @param flags
     * @return The file descriptor for the file just opened
     */
    void syscall_open(const char *path, int flags);

    /**
     * @brief Close a proviously open file
     * @param fd File descriptor of the open file to close
     * @return
     */
    void syscall_close(int fd);

    /**
     * @brief Create a new file at the given path
     * @param path Path of the file to create
     * @param flags Mode for the new file
     * @return
     */
    void syscall_create(const char *path, int flags);

    /**
     * @brief
     * @param fd
     * @return
     */
    void syscall_unlink(int fd);

    /**
     * @brief
     * @param fd
     * @param buffer
     * @param size
     * @return
     */
    void syscall_read(int fd, void *buffer, unsigned long size);

    /**
     * @brief
     * @param fd
     * @param buffer
     * @param size
     * @return
     */
    void syscall_write(int fd, const void *buffer, unsigned long size);

    /**
     * @brief Duplicate a file descriptor. `newfd` will serve as an alias for
     * `oldfd`.
     * @param oldfd
     * @param newfd
     */
    void syscall_fddup(int oldfd, int newfd);

    /**
     * @brief Creates a new pipe.
     */
    void syscall_create_pipe(int pipefd[2]);
}

#endif