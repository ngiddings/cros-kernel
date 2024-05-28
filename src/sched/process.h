#ifndef KERNEL_PROCESS_H
#define KERNEL_PROCESS_H

#include "memory/addressspace.h"
#include "context.h"
#include "types/pid.h"
#include "signalaction.h"
#include "containers/binary_search_tree.h"
#include "fs/filecontext.h"

namespace kernel::sched
{

    class Process
    {
    public:
        enum class State
        {
            ACTIVE,
            SIGNAL,
            SIGWAIT
        };

        static pid_t nextPid();

        Process();

        Process(pid_t pid, pid_t parent, void *entry, void *stack, void *kernelStack, kernel::memory::AddressSpace *addressSpace);

        Process(Process &other);

        ~Process();

        Process &operator=(Process &other);

        int exec(void *pc, void *stack, void *kernelStack, kernel::memory::AddressSpace *addressSpace);

        Process *clone(pid_t pid, void *pc, void *stack, void *userdata);

        Context *getContext();

        void storeContext(Context *newCtx);

        State getState() const;

        void setState(State newState);

        pid_t getPid() const;

        pid_t getParent() const;

        kernel::memory::AddressSpace *getAddressSpace();

        void setSignalAction(int signal, void (*handler)(void *), void (*trampoline)(void), void *userdata);

        int signalTrigger(int sig);

        void signalReturn();

        void storeProgramArgs(char *const argv[], char *const envp[]);

        kernel::fs::FileContext *getFileContext(int fd) const;

        int storeFileContext(kernel::fs::FileContext *f);

        int storeFileContext(kernel::fs::FileContext *f, int fd);

        int closeFileContext(int fd);

    private:
        static pid_t nextPidVal;

        static const int MAX_SIGNAL = 64;

        pid_t pid, parent;

        Context ctx, *backupCtx;

        State state;

        kernel::memory::AddressSpace *addressSpace;

        SignalAction signalHandlers[MAX_SIGNAL];

        binary_search_tree<int, kernel::fs::FileContext *> files;
    };

}

#endif