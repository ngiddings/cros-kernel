#ifndef KERNEL_CONTEXT_H
#define KERNEL_CONTEXT_H

#include <cstdint>

namespace kernel::sched
{

    class Context
    {
    public:
        Context();

        Context(void *pc);

        Context(void *pc, void *sp);

        Context(void *pc, void *sp, void *ksp);

        void functionCall(void *func_ptr, void *returnLoc, unsigned long arg);

        void setProgramCounter(void *pc);

        void *getProgramCounter() const;

        void setStackPointer(void *sp);

        void *getStackPointer() const;

        void setKernelStack(void *sp);

        void *getKernelStack() const;

        void setProcessArgs(int argc, char **argv, char **envp);

        void setReturnValue(unsigned long v);

        void pushLong(unsigned long v);

        void pushString(const char *str);

    private:
#if defined __aarch64__

        uint64_t fpRegs[64];

        uint64_t gpRegs[31];

        uint64_t stackPointer;

        uint64_t programCounter;

        uint64_t programStatus;

        uint64_t fpcr;

        uint64_t fpsr;

        uint64_t kernelStack;

#else
#error "Platform not supported"
#endif
    };

    extern "C" void load_context(const Context *ctx);

}

#endif