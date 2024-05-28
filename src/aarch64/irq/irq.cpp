#include <cstdint>
#include "irq/interrupts.h"
#include "util/hacf.h"
#include "exceptionclass.h"
#include "syndromedataabort.h"
#include "devices/mmio.h"
#include "sched/context.h"
#include "../sysreg.h"
#include "util/log.h"
#include "kernel.h"

void handlePageFault(ExceptionClass type, SyndromeDataAbort syndrome);

extern "C" int find_irq_source()
{
    unsigned int pending1 = mmio_read((void *)MMIOOffset::INTR_IRQ_PENDING_1);
    if (pending1 != 0)
    {
        return __builtin_ctz(pending1);
    }

    unsigned int pending2 = mmio_read((void *)MMIOOffset::INTR_IRQ_PENDING_2);
    if (pending2 != 0)
    {
        return 32 + __builtin_ctz(pending2);
    }

    unsigned int base = mmio_read((void *)MMIOOffset::INTR_BASIC_PENDING) & 255;
    if (base != 0)
    {
        return __builtin_ctz(base);
    }

    return -1;
}

extern "C" kernel::sched::Context *handle_sync(ExceptionClass type, unsigned long syndrome, kernel::sched::Context *ctx)
{
    switch (type)
    {
    case ExceptionClass::INST_ABORT_EL0:
        kernelLog(LogLevel::PANIC, "Unhandled INST_ABORT_EL0, FAR_EL1 = %016x", get_far_el1());
        hacf();
    case ExceptionClass::INST_ABORT_EL1:
        kernelLog(LogLevel::PANIC, "Unhandled INST_ABORT_EL1, FAR_EL1 = %016x", get_far_el1());
        hacf();
    case ExceptionClass::DATA_ABORT_EL0:
        kernelLog(LogLevel::PANIC, "Unhandled DATA_ABORT_EL0, FAR_EL1 = %016x", get_far_el1());
        hacf();
        break;
    case ExceptionClass::DATA_ABORT_EL1:
        handlePageFault(type, *(SyndromeDataAbort *)&syndrome);
        break;
    case ExceptionClass::SVC_AARCH64:
    case ExceptionClass::SVC_AARCH32:
        break;
    default:
        kernelLog(LogLevel::PANIC, "Unimplemented exception class");
        hacf();
        break;
    }
    return ctx;
}

extern "C" kernel::sched::Context *handle_irq(int source, kernel::sched::Context *ctx)
{
    using namespace kernel::interrupt;
    if (kernel::kernel.getActiveProcess() != nullptr)
    {
        kernel::kernel.getActiveProcess()->storeContext(ctx);
    }

    // kernelLog(LogLevel::DEBUG, "handle_irq(%i, %016x)", source, ctx);

    if (source < 0)
    {
        return ctx;
    }

    Interrupts::callHandler(source);
    if (kernel::kernel.getActiveProcess() != nullptr)
    {
        return kernel::kernel.getActiveProcess()->getContext();
    }
    return ctx;
}
