#include "sched/context.h"
#include "util/string.h"

kernel::sched::Context::Context()
{
    for (int i = 0; i < 64; i++)
    {
        fpRegs[i] = 0;
    }
    for (int i = 0; i < 31; i++)
    {
        gpRegs[i] = i;
    }
    stackPointer = 0;
    programCounter = 0;
    programStatus = 0;
    fpcr = 0;
    fpsr = 0;
}

kernel::sched::Context::Context(void *pc)
{
    for (int i = 0; i < 64; i++)
    {
        fpRegs[i] = 0;
    }
    for (int i = 0; i < 31; i++)
    {
        gpRegs[i] = i;
    }
    stackPointer = 0;
    programCounter = (uint64_t)pc;
    programStatus = 0;
    fpcr = 0;
    fpsr = 0;
    kernelStack = 0;
}

kernel::sched::Context::Context(void *pc, void *sp)
{
    for (int i = 0; i < 64; i++)
    {
        fpRegs[i] = 0;
    }
    for (int i = 0; i < 31; i++)
    {
        gpRegs[i] = i;
    }
    stackPointer = (uint64_t)sp;
    programCounter = (uint64_t)pc;
    programStatus = 0;
    fpcr = 0;
    fpsr = 0;
    kernelStack = 0;
}

kernel::sched::Context::Context(void *pc, void *sp, void *ksp)
{
    for (int i = 0; i < 64; i++)
    {
        fpRegs[i] = 0;
    }
    for (int i = 0; i < 31; i++)
    {
        gpRegs[i] = i;
    }
    stackPointer = (uint64_t)sp;
    programCounter = (uint64_t)pc;
    programStatus = 0;
    fpcr = 0;
    fpsr = 0;
    kernelStack = (uint64_t)ksp;
}

void kernel::sched::Context::functionCall(void *func_ptr, void *returnLoc, unsigned long arg)
{
    programCounter = (unsigned long)func_ptr;
    gpRegs[30] = (unsigned long)returnLoc;
    gpRegs[0] = arg;
}

void kernel::sched::Context::setProgramCounter(void *pc)
{
    programCounter = (uint64_t)pc;
}

void *kernel::sched::Context::getProgramCounter() const
{
    return (void *)programCounter;
}

void kernel::sched::Context::setStackPointer(void *sp)
{
    stackPointer = (uint64_t)sp;
}

void *kernel::sched::Context::getStackPointer() const
{
    return (void *)stackPointer;
}

void kernel::sched::Context::setKernelStack(void *sp)
{
    kernelStack = (uint64_t)sp;
}

void *kernel::sched::Context::getKernelStack() const
{
    return (void *)kernelStack;
}

void kernel::sched::Context::setProcessArgs(int argc, char **argv, char **envp)
{
    gpRegs[0] = (uint64_t)argc;
    gpRegs[1] = (uint64_t)argv;
    gpRegs[2] = (uint64_t)envp;
}

void kernel::sched::Context::setReturnValue(unsigned long v)
{
    gpRegs[0] = v;
}

void kernel::sched::Context::pushLong(unsigned long v)
{
    unsigned long *sp = (unsigned long *)stackPointer;
    *--sp = v;
    stackPointer = (unsigned long)sp;
}

void kernel::sched::Context::pushString(const char *str)
{
    int len = strlen(str) + 1;
    len += 15;
    len -= len % 16;
    stackPointer -= len;
    strcpy((char *)stackPointer, str);
}
