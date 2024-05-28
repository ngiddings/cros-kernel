#include "process.h"
#include "memory/new.h"
#include "memory/mmap.h"
#include "types/status.h"
#include "util/log.h"
#include "memory/heap.h"

pid_t kernel::sched::Process::nextPidVal = 1;

pid_t kernel::sched::Process::nextPid()
{
    pid_t v = nextPidVal;
    nextPidVal++;
    return v;
}

kernel::sched::Process::Process()
    : pid(0), parent(0), state(State::ACTIVE), ctx(), addressSpace(nullptr), backupCtx(nullptr), files()
{
    for (int i = 0; i < MAX_SIGNAL; i++)
    {
        signalHandlers[i].type = ActionType::NONE;
    }
}

kernel::sched::Process::Process(pid_t pid, pid_t parent, void *entry, void *stack, void *kernelStack, kernel::memory::AddressSpace *addressSpace)
    : pid(pid), parent(parent), state(State::ACTIVE), ctx(entry, stack, kernelStack), addressSpace(addressSpace), backupCtx(nullptr), files()
{
    addressSpace->addReference();
    for (int i = 0; i < MAX_SIGNAL; i++)
    {
        signalHandlers[i].type = ActionType::NONE;
    }
}

kernel::sched::Process::Process(Process &other)
    : files()
{
    pid = other.pid;
    parent = other.parent;
    ctx = other.ctx;
    state = other.state;
    addressSpace = other.addressSpace;
    if (addressSpace != nullptr)
    {
        addressSpace->addReference();
    }
    if (other.backupCtx == nullptr)
    {
        backupCtx = nullptr;
    }
    else
    {
        backupCtx = new Context(*other.backupCtx);
    }
    for (int i = 0; i < MAX_SIGNAL; i++)
    {
        signalHandlers[i].type = other.signalHandlers[i].type;
        signalHandlers[i].handler = other.signalHandlers[i].handler;
        signalHandlers[i].trampoline = other.signalHandlers[i].trampoline;
        signalHandlers[i].userdata = other.signalHandlers[i].userdata;
    }
    for (int fd : other.files)
    {
        storeFileContext(other.files.get(fd), fd);
    }
}

kernel::sched::Process::~Process()
{
    addressSpace->removeReference();
    if (addressSpace->getRefCount() <= 0)
    {
        kernel::memory::destoryAddressSpace(*addressSpace);
        delete addressSpace;
    }
    if (backupCtx != nullptr)
    {
        delete backupCtx;
    }
    for (int fd : files)
    {
        closeFileContext(fd);
    }
}

int kernel::sched::Process::exec(void *pc, void *stack, void *kernelStack, kernel::memory::AddressSpace *addressSpace)
{
    if (state != State::ACTIVE)
    {
        // This will probably break if called during a signal handler
        return -1;
    }
    if (this->addressSpace != nullptr)
    {
        this->addressSpace->removeReference();
        if (this->addressSpace->getRefCount() <= 0)
        {
            kernel::memory::destoryAddressSpace(*this->addressSpace);
            delete this->addressSpace;
        }
    }
    this->addressSpace = addressSpace;
    this->addressSpace->addReference();
    ctx.setProgramCounter(pc);
    ctx.setStackPointer(stack);
    ctx.setKernelStack(kernelStack);
    return 0;
}

kernel::sched::Process *kernel::sched::Process::clone(pid_t pid, void *pc, void *stack, void *userdata)
{
    using namespace kernel::fs;

    void *base = rmalloc(1UL << 16);
    if (base == nullptr)
    {
        return nullptr;
    }

    void *kernelStack = (void *)((unsigned long)base + (1UL << 16));
    Process *copy = new Process(pid, this->pid, pc, stack, kernelStack, this->addressSpace);
    if (copy == nullptr)
    {
        return nullptr;
    }
    copy->getContext()->functionCall(pc, nullptr, (unsigned long)userdata);

    for (int fd : files)
    {
        copy->storeFileContext(files.get(fd)->copy(), fd);
    }

    return copy;
}

kernel::sched::Context *kernel::sched::Process::getContext()
{
    return &ctx;
}

void kernel::sched::Process::storeContext(kernel::sched::Context *newCtx)
{
    ctx = *newCtx;
}

kernel::sched::Process::State kernel::sched::Process::getState() const
{
    return state;
}

void kernel::sched::Process::setState(State newState)
{
    state = newState;
}

pid_t kernel::sched::Process::getPid() const
{
    return pid;
}

pid_t kernel::sched::Process::getParent() const
{
    return parent;
}

kernel::memory::AddressSpace *kernel::sched::Process::getAddressSpace()
{
    return addressSpace;
}

void kernel::sched::Process::setSignalAction(int signal, void (*handler)(void *), void (*trampoline)(void), void *userdata)
{
    if (signal < 0 || signal >= MAX_SIGNAL)
    {
        return;
    }
    if (handler == nullptr)
    {
        signalHandlers[signal].type = ActionType::NONE;
        signalHandlers[signal].handler = nullptr;
        signalHandlers[signal].trampoline = nullptr;
        signalHandlers[signal].userdata = nullptr;
    }
    else
    {
        signalHandlers[signal].type = ActionType::HANDLER;
        signalHandlers[signal].handler = handler;
        signalHandlers[signal].trampoline = trampoline;
        signalHandlers[signal].userdata = userdata;
    }
}

int kernel::sched::Process::signalTrigger(int sig)
{
    if (sig < 0 || sig >= MAX_SIGNAL)
    {
        return 0;
    }

    switch (signalHandlers[sig].type)
    {
    case ActionType::NONE:
        return 0;
    case ActionType::HANDLER:
        if (state == State::SIGNAL)
        {
            return -1;
        }
        backupCtx = new Context(ctx);
        ctx.functionCall((void *)signalHandlers[sig].handler,
                         (void *)signalHandlers[sig].trampoline,
                         (unsigned long)signalHandlers[sig].userdata);
        state = State::SIGNAL;
        return 0;
    case ActionType::KILL:
        return 1;
    }
}

void kernel::sched::Process::signalReturn()
{
    if (state != State::SIGNAL)
    {
        return;
    }
    ctx = *backupCtx;
    delete backupCtx;
    backupCtx = nullptr;
    state = State::ACTIVE;
}

void kernel::sched::Process::storeProgramArgs(char *const argv[], char *const envp[])
{
    int envc = 0;
    while (envp[envc] != nullptr)
    {
        envc++;
    }

    int argc = 0;
    while (argv[argc] != nullptr)
    {
        argc++;
    }

    char **argArray = new char *[argc];
    char **envArray = new char *[envc + 1];
    for (int i = argc - 1; i >= 0; i--)
    {
        ctx.pushString(argv[i]);
        argArray[i] = (char *)ctx.getStackPointer();
    }
    for (int i = envc - 1; i >= 0; i--)
    {
        ctx.pushString(envp[i]);
        envArray[i] = (char *)ctx.getStackPointer();
    }
    envArray[envc] = nullptr;

    if (argc % 2 == 1)
    {
        ctx.pushLong(0);
    }
    for (int i = argc - 1; i >= 0; i--)
    {
        ctx.pushLong((unsigned long)argArray[i]);
    }
    void *argPtr = ctx.getStackPointer();

    if ((envc + 1) % 2 == 1)
    {
        ctx.pushLong(0);
    }
    for (int i = envc; i >= 0; i--)
    {
        ctx.pushLong((unsigned long)envArray[i]);
    }
    void *envPtr = ctx.getStackPointer();

    ctx.setProcessArgs(argc, (char **)argPtr, (char **)envPtr);
}

kernel::fs::FileContext *kernel::sched::Process::getFileContext(int fd) const
{
    if (files.contains(fd))
    {
        return files.get(fd);
    }
    else
    {
        return nullptr;
    }
}

int kernel::sched::Process::storeFileContext(kernel::fs::FileContext *f)
{
    int fd = files.size();
    files.insert(fd, f);
    f->addReference();
    return fd;
}

int kernel::sched::Process::storeFileContext(kernel::fs::FileContext *f, int fd)
{
    if (files.contains(fd))
    {
        return EEXISTS;
    }
    else
    {
        files.insert(fd, f);
        f->addReference();
        return ENONE;
    }
}

int kernel::sched::Process::closeFileContext(int fd)
{
    using namespace kernel::fs;
    if (!files.contains(fd))
    {
        return ENOFILE;
    }
    FileContext *fc = files.get(fd);
    files.remove(fd);
    fc->removeReference();
    if (fc->getRefCount() <= 0)
    {
        // kernelLog(LogLevel::DEBUG, "Freeing file context %i", fd);
        delete fc;
    }
    return ENONE;
}

kernel::sched::Process &kernel::sched::Process::operator=(Process &other)
{
    pid = other.pid;
    parent = other.parent;
    ctx = other.ctx;
    state = other.state;
    if (addressSpace != nullptr && addressSpace != other.addressSpace)
    {
        addressSpace->removeReference();
        if (addressSpace->getRefCount() <= 0)
        {
            kernel::memory::destoryAddressSpace(*addressSpace);
            delete addressSpace;
        }
    }
    addressSpace = other.addressSpace;
    if (addressSpace != nullptr)
    {
        addressSpace->addReference();
    }
    if (other.backupCtx == nullptr)
    {
        backupCtx = nullptr;
    }
    else
    {
        backupCtx = new Context(*other.backupCtx);
    }
    for (int i = 0; i < MAX_SIGNAL; i++)
    {
        signalHandlers[i].type = other.signalHandlers[i].type;
        signalHandlers[i].handler = other.signalHandlers[i].handler;
        signalHandlers[i].trampoline = other.signalHandlers[i].trampoline;
        signalHandlers[i].userdata = other.signalHandlers[i].userdata;
    }
    for (int fd : files)
    {
        closeFileContext(fd);
    }
    for (int fd : other.files)
    {
        storeFileContext(other.files.get(fd), fd);
    }
    return *this;
}
