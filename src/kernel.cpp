#include "kernel.h"
#include "util/log.h"
#include "util/hacf.h"
#include "util/string.h"
#include "mmgmt.h"
#include "types/physaddr.h"
#include "sched/process.h"
#include "loader/elf.h"
#include "fs/fat32/filecontextfat32.h"
#include "types/status.h"
#include "fs/pipe.h"

kernel::Kernel kernel::kernel;

class PipeReadContext : public kernel::fs::FileContext
{
    int read(void *buffer, int n);

    int write(const void *buffer, int n);
};

void (*syscall_table[])(long, long, long, long) = {
    (void (*)(long, long, long, long))kernel::syscall_printk,
    (void (*)(long, long, long, long))kernel::syscall_mmap,
    (void (*)(long, long, long, long))kernel::syscall_munmap,
    (void (*)(long, long, long, long))kernel::syscall_clone,
    (void (*)(long, long, long, long))kernel::syscall_terminate,
    (void (*)(long, long, long, long))kernel::syscall_exec,
    (void (*)(long, long, long, long))kernel::syscall_yield,
    (void (*)(long, long, long, long))kernel::syscall_sigraise,
    (void (*)(long, long, long, long))kernel::syscall_sigret,
    (void (*)(long, long, long, long))kernel::syscall_sigwait,
    (void (*)(long, long, long, long))kernel::syscall_sigaction,
    (void (*)(long, long, long, long))kernel::syscall_open,
    (void (*)(long, long, long, long))kernel::syscall_close,
    (void (*)(long, long, long, long))kernel::syscall_create,
    (void (*)(long, long, long, long))kernel::syscall_unlink,
    (void (*)(long, long, long, long))kernel::syscall_read,
    (void (*)(long, long, long, long))kernel::syscall_write,
    (void (*)(long, long, long, long))kernel::syscall_fddup,
    (void (*)(long, long, long, long))kernel::syscall_create_pipe};

kernel::sched::Context *do_syscall(unsigned long id, unsigned long arg1, unsigned long arg2, unsigned long arg3, unsigned long arg4, kernel::sched::Context *ctx)
{
    using namespace kernel::sched;
    kernel::kernel.getActiveProcess()->storeContext(ctx);
    syscall_table[id](arg1, arg2, arg3, arg4);
    // kernelLog(LogLevel::DEBUG, "Returning from call %i:\n\tpc = %016x\n\tsp=%016x", id, ctx->getProgramCounter(), ctx->getStackPointer());
    return kernel::kernel.getActiveProcess()->getContext();
}

void kernel::syscall_printk(const char *str)
{
    printf(str);
    kernel.setCallerReturn(ENONE);
}

void kernel::syscall_mmap(void *ptr, unsigned long size, int flags)
{
    using namespace kernel::memory;
    physaddr_t frame = pageAllocator.reserve(size);
    if (frame == PageAllocator::NOMEM)
    {
        kernel.setCallerReturn(ENOMEM);
    }
    else
    {
        kernel.setCallerReturn(map_region(ptr, size, frame, flags | PAGE_USER));
    }
}

void kernel::syscall_munmap(void *ptr, unsigned long size)
{
    using namespace kernel::memory;
    unmap_region(ptr, size);
    kernel.setCallerReturn(ENONE);
}

void kernel::syscall_clone(int (*fn)(void *), void *stack, void *userdata, int flags)
{
    using namespace kernel::sched;
    Process *newProcess = kernel.getActiveProcess()->clone(kernel.nextPid(), (void *)fn, stack, userdata);

    kernel.addProcess(*newProcess);
    kernel.setCallerReturn(ENONE);
}

void kernel::syscall_terminate()
{
    kernel.raiseSignal(kernel.getActiveProcess()->getParent(), 17);
    kernel.deleteActiveProcess();
    kernel.switchTask();
}

void kernel::syscall_exec(const char *path, char *const argv[], char *const envp[])
{
    if (path == nullptr || argv == nullptr || envp == nullptr)
    {
        kernel.setCallerReturn(EINVAL);
        return;
    }

    int argc = 0;
    while (argv[argc] != nullptr)
    {
        argc++;
    }

    int envc = 0;
    while (envp[envc] != nullptr)
    {
        envc++;
    }

    char **argvCopy = new char *[argc + 1];
    for (int i = 0; i <= argc; i++)
    {
        if (argv[i] != nullptr)
        {
            argvCopy[i] = new char[strlen(argv[i]) + 1];
            strcpy(argvCopy[i], argv[i]);
        }
        else
        {
            argvCopy[i] = nullptr;
        }
    }

    char **envpCopy = new char *[envc + 1];
    for (int i = 0; i <= envc; i++)
    {
        if (envp[i] != nullptr)
        {
            envpCopy[i] = new char[strlen(envp[i]) + 1];
            strcpy(envpCopy[i], envp[i]);
        }
        else
        {
            envpCopy[i] = nullptr;
        }
    }

    char *pathCopy = (char *)rmalloc(strlen(path) + 1);
    strcpy(pathCopy, path);

    int status;
    if ((status = kernel.exec(path, argvCopy, envpCopy)) != ENONE)
    {
        kernel.setCallerReturn(status);
    }

    for (int i = 0; argvCopy[i] != nullptr; i++)
    {
        delete argvCopy[i];
    }

    for (int i = 0; envpCopy[i] != nullptr; i++)
    {
        delete envpCopy[i];
    }

    delete envpCopy;
    delete argvCopy;
    // kernelLog(LogLevel::DEBUG, "Returning exec() into %s", pathCopy);
    rfree(pathCopy);
}

void kernel::syscall_yield()
{
    kernel.setCallerReturn(ENONE);
    kernel.switchTask();
}

void kernel::syscall_sigraise(pid_t pid, int signal)
{
    int status = kernel.raiseSignal(pid, signal);
    if (status <= 0)
    {
        kernel.setCallerReturn(status);
    }
}

void kernel::syscall_sigret()
{
    kernel.getActiveProcess()->signalReturn();
}

void kernel::syscall_sigwait()
{
    using namespace kernel::sched;
    kernel.getActiveProcess()->setState(Process::State::SIGWAIT);
    kernel.sleepActiveProcess();
    kernel.switchTask();
}

void kernel::syscall_sigaction(int signal, void (*handler)(void *), void (*trampoline)(void), void *userdata)
{
    kernel.getActiveProcess()->setSignalAction(signal, handler, trampoline, userdata);
    kernel.setCallerReturn(ENONE);
}

void kernel::syscall_open(const char *path, int flags)
{
    using namespace kernel::fs;
    if (kernel.getRamFS() == nullptr)
    {
        kernel.setCallerReturn(EIO);
        return;
    }

    FileType type;
    if (kernel.getRamFS()->file_type(path, type) == failure || type != File)
    {
        kernel.setCallerReturn(ENOFILE);
        return;
    }

    FileContext *fc = new FileContextFAT32(*kernel.getRamFS(), path);
    int fd = kernel.getActiveProcess()->storeFileContext(fc);
    kernel.setCallerReturn(fd);
}

void kernel::syscall_close(int fd)
{
    kernel.setCallerReturn(kernel.getActiveProcess()->closeFileContext(fd));
}

void kernel::syscall_create(const char *path, int flags)
{
    kernel.setCallerReturn(ENOSYS);
}

void kernel::syscall_unlink(int fd)
{
    kernel.setCallerReturn(ENOSYS);
}

void kernel::syscall_read(int fd, void *buffer, unsigned long size)
{
    using namespace kernel::fs;
    // kernelLog(LogLevel::DEBUG, "pid %i: read(%i, %016x, %i)", kernel.getActiveProcess()->getPid(), fd, buffer, size);
    FileContext *fc = kernel.getActiveProcess()->getFileContext(fd);
    if (fc == nullptr)
    {
        kernel.setCallerReturn(ENOFILE);
        return;
    }

    int count = fc->read(buffer, size);
    kernel.setCallerReturn(count);
}

void kernel::syscall_write(int fd, const void *buffer, unsigned long size)
{
    using namespace kernel::fs;
    // kernelLog(LogLevel::DEBUG, "pid %i: write(%i, %016x, %i)", kernel.getActiveProcess()->getPid(), fd, buffer, size);
    FileContext *fc = kernel.getActiveProcess()->getFileContext(fd);
    if (fc == nullptr)
    {
        kernelLog(LogLevel::WARNING, "Failed to write on fd %i", fd);
        kernel.setCallerReturn(ENOFILE);
        return;
    }
    kernel.setCallerReturn(fc->write(buffer, size));
}

void kernel::syscall_fddup(int oldfd, int newfd)
{
    using namespace kernel::fs;
    // kernelLog(LogLevel::DEBUG, "pid %i: fddup(%i, %i)", kernel.getActiveProcess()->getPid(), oldfd, newfd);
    FileContext *fc = kernel.getActiveProcess()->getFileContext(oldfd);
    if (fc == nullptr)
    {
        kernel.setCallerReturn(ENOFILE);
        return;
    }
    if (kernel.getActiveProcess()->getFileContext(newfd) != nullptr && kernel.getActiveProcess()->closeFileContext(newfd))
    {
        kernelLog(LogLevel::ERROR, "fddup() failed to close newfd=%i", newfd);
        kernel.setCallerReturn(EUNKNOWN);
        return;
    }
    if (kernel.getActiveProcess()->storeFileContext(fc, newfd) != ENONE)
    {
        kernelLog(LogLevel::ERROR, "fddup() failed to store newfd=%i", newfd);
        kernel.setCallerReturn(EUNKNOWN);
        return;
    }
    kernel.setCallerReturn(ENONE);
}

void kernel::syscall_create_pipe(int pipefd[2])
{
    using namespace kernel::fs;
    // kernelLog(LogLevel::DEBUG, "pid %i: pipe", kernel.getActiveProcess()->getPid());
    Pipe *pipe = new Pipe();
    if (pipe == nullptr)
    {
        kernel.setCallerReturn(ENOMEM);
        return;
    }

    FileContext *reader = pipe->createReader();
    FileContext *writer = pipe->createWriter();
    if (reader == nullptr || writer == nullptr)
    {
        if (reader)
        {
            delete reader;
        }
        if (writer)
        {
            delete writer;
        }
        kernel.setCallerReturn(ENOMEM);
        return;
    }

    pipefd[0] = kernel.getActiveProcess()->storeFileContext(reader);
    pipefd[1] = kernel.getActiveProcess()->storeFileContext(writer);
    kernel.setCallerReturn(ENONE);
}

kernel::Kernel::Kernel()
    : scheduler(), processTable(), fs(nullptr), currPid(1)
{
}

pid_t kernel::Kernel::nextPid()
{
    pid_t v = currPid;
    currPid++;
    return v;
}

int kernel::Kernel::initMemory(memory::MemoryMap &memoryMap, unsigned long kernelSize)
{
    using namespace memory;
    kernelLog(LogLevel::DEBUG, "Constructing page allocator at %016x", &__end);
    new (&pageAllocator) memory::PageAllocator(memoryMap, &__end, page_size);
    unsigned long pageMapEnd = (unsigned long)&__end + PageAllocator::mapSize(memoryMap, page_size);
    pageMapEnd = (pageMapEnd + (page_size - 1)) & ~(page_size - 1);
    unsigned long heapSize = ((unsigned long)&__high_mem + kernelSize) - pageMapEnd;

    kernelLog(LogLevel::DEBUG, "Constructing kernel heap at %016x with size %016x", pageMapEnd, heapSize);
    init_heap((void *)pageMapEnd, heapSize);
    return 0;
}

int kernel::Kernel::initRamFS(void *ramfs)
{
    kernelLog(LogLevel::DEBUG, "Creating ramfs %016x", ramfs);
    if (fs != nullptr)
    {
        kernelLog(LogLevel::INFO, "Deleting previous ramfs %016x", fs);
        delete fs;
    }
    fs = new FAT32(ramfs);
    return 0;
}

FAT32 *kernel::Kernel::getRamFS()
{
    return fs;
}

void kernel::Kernel::switchTask()
{
    scheduler.sched_next();
    memory::loadAddressSpace(*scheduler.get_cur_process()->getAddressSpace());
    // kernelLog(LogLevel::DEBUG, "Switched to pid %i", getActiveProcess()->getPid());
}

void kernel::Kernel::setCallerReturn(unsigned long v)
{
    scheduler.get_cur_process()->getContext()->setReturnValue(v);
}

void kernel::Kernel::addProcess(sched::Process &p)
{
    using namespace sched;
    processTable.insert(p.getPid(), p);
    if (p.getState() == Process::State::ACTIVE)
    {
        scheduler.enqueue(&processTable.get(p.getPid()));
    }
}

kernel::sched::Process *kernel::Kernel::getActiveProcess()
{
    return scheduler.get_cur_process();
}

void kernel::Kernel::sleepActiveProcess()
{
    scheduler.set_cur_process(nullptr);
}

void kernel::Kernel::deleteActiveProcess()
{
    processTable.remove(scheduler.get_cur_process()->getPid());
    scheduler.set_cur_process(nullptr);
}

int kernel::Kernel::raiseSignal(pid_t pid, int signal)
{
    using namespace kernel::sched;
    if (!processTable.contains(pid))
    {
        kernelLog(LogLevel::WARNING, "Attempt to raise signal %i on non-existent pid %i", signal, pid);
        return -1;
    }
    else if (processTable.get(pid).getState() != Process::State::ACTIVE && processTable.get(pid).getState() != Process::State::SIGWAIT)
    {
        kernelLog(LogLevel::WARNING, "Process %i cannot acccept signal: invalid state.", pid);
        return -1;
    }

    bool schedule = processTable.get(pid).getState() == Process::State::SIGWAIT;
    int status = processTable.get(pid).signalTrigger(signal);
    if (status > 0)
    {
        kernelLog(LogLevel::DEBUG, "Killing process %i due to signal.", pid);
        if (processTable.get(pid).getState() == Process::State::ACTIVE)
        {
            scheduler.remove(pid);
            if (getActiveProcess()->getPid() == pid)
            {
                scheduler.set_cur_process(nullptr);
                switchTask();
            }
        }
        processTable.remove(pid);
    }
    else if (status == 0 && schedule)
    {
        // kernelLog(LogLevel::DEBUG, "Placing process %i back on schedule queue.", pid);
        scheduler.enqueue(&processTable.get(pid));
    }
    return status;
}

int kernel::Kernel::exec(const char *path, char *const argv[], char *const envp[])
{
    using namespace memory;
    using namespace loader;
    using namespace sched;
    using namespace fs;

    // kernelLog(LogLevel::DEBUG, "exec %s", path);

    if (getActiveProcess() == nullptr || getActiveProcess()->getState() != Process::State::ACTIVE || argv == nullptr || envp == nullptr)
    {
        return -1;
    }

    FileType type;
    int size;
    if (fs->file_type(path, type) == failure || fs->file_size(path, size) == failure)
    {
        kernelLog(LogLevel::INFO, "kernel.exec() failure: %s not found.", path);
        return ENOFILE;
    }

    void *exeData = rmalloc(size);
    for (int i = 0; i < size; i += 512)
    {
        byte *data;
        if (fs->read_file(path, i / 512, data) == failure)
        {
            kernelLog(LogLevel::WARNING, "kernel.exec() failure: I/O error.");
            return EIO;
        }
        memcpy((char *)exeData + i, data, size - i >= 512 ? 512 : size - i);
        delete data;
        data = nullptr;
    }

    AddressSpace *addressSpace = createAddressSpace();
    loadAddressSpace(*addressSpace);

    ELF exe(exeData);
    buildProgramImage(exe);

    map_region((void *)0x7FBFFF0000, 0x10000, pageAllocator.reserve(0x10000), PAGE_USER | PAGE_RW);
    void *base = rmalloc(1UL << 16);
    if (base == nullptr)
    {
        rfree(exeData);
        return ENOMEM;
    }
    void *kernelStack = (void *)((unsigned long)base + (1UL << 16));

    getActiveProcess()->exec(exe.fileHeader().entry, (void *)0x7FC0000000, kernelStack, addressSpace);
    getActiveProcess()->storeProgramArgs(argv, envp);

    if (getActiveProcess()->getFileContext(0) == nullptr)
    {
        FileContext *in = getLogStream()->open(CharStream::Mode::RO);
        if (in != nullptr)
        {
            if (getActiveProcess()->storeFileContext(in, 0))
            {
                delete in;
            }
        }
    }

    if (getActiveProcess()->getFileContext(1) == nullptr)
    {
        FileContext *out = getLogStream()->open(CharStream::Mode::W);
        if (out != nullptr)
        {
            if (getActiveProcess()->storeFileContext(out, 1))
            {
                delete out;
            }
        }
    }

    if (getActiveProcess()->getFileContext(1) == nullptr)
    {
        FileContext *out = getLogStream()->open(CharStream::Mode::W);
        if (out != nullptr)
        {
            if (getActiveProcess()->storeFileContext(out, 1))
            {
                delete out;
            }
        }
    }

    if (getActiveProcess()->getFileContext(2) == nullptr)
    {
        FileContext *err = getLogStream()->open(CharStream::Mode::W);
        if (err != nullptr)
        {
            if (getActiveProcess()->storeFileContext(err, 2))
            {
                delete err;
            }
        }
    }

    rfree(exeData);
    return ENONE;
}
