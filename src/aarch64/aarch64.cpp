#include <cstdint>
#include "mmgmt.h"
#include "sysreg.h"
#include "irq/exceptions.h"
#include "util/string.h"
#include "devices/uart.h"
#include "devices/timer.h"
#include "util/log.h"
#include "sched/context.h"
#include "irq/interrupts.h"
#include "loader/elf.h"
#include "util/hacf.h"
#include "sched/process.h"
#include "kernel.h"
#include "fs/fat32/fat32.h"
#include "containers/binary_search_tree.h"
#include "util/charstream.h"

using namespace kernel::memory;
using namespace kernel::devices;
using namespace kernel::sched;
using namespace kernel::interrupt;
using namespace kernel::loader;
using namespace kernel::fs;

UART uart;

SystemTimer timer;

extern "C" void aarch64_boot(uint64_t dtb, uint64_t kernelSize)
{
    Interrupts::init();
    // Construct UART driver
    new (&uart) UART((void *)0xFFFFFF803F201000);
    Interrupts::insertHandler(57, &uart);

    // Initialize log
    logInit(&uart);

    kernelLog(LogLevel::INFO, "CROS startup...");
    kernelLog(LogLevel::DEBUG, "DTB Location = %016x", dtb);
    kernelLog(LogLevel::DEBUG, "Kernel size = %i MiB", kernelSize >> 20);

    MemoryMap map;
    map.place(MemoryMap::MemoryType::AVAILABLE, 0, 0x20000000);
    map.place(MemoryMap::MemoryType::MMIO, 0x3f000000, 0x1000000);
    map.place(MemoryMap::MemoryType::UNAVAILABLE, (unsigned long)0, kernelSize);
    map.place(MemoryMap::MemoryType::UNAVAILABLE, (unsigned long)&__end - (unsigned long)&__high_mem, PageAllocator::mapSize(map, page_size));
    map.place(MemoryMap::MemoryType::UNAVAILABLE, (unsigned long)0x8000000, 1 << 26);
    kernel::kernel.initMemory(map, kernelSize);

    setPageEntry(2, &__high_mem + 0x100000000, 0, PAGE_RW);
    kernel::kernel.initRamFS((void *)(&__high_mem + 0x108000000));

    new (&timer) SystemTimer(50);
    Interrupts::insertHandler(0, &timer);
    Interrupts::insertHandler(1, &timer);
    Interrupts::insertHandler(2, &timer);
    Interrupts::insertHandler(3, &timer);

    char *const argv[] = {"/bin/init", nullptr};
    char *const envp[] = {"cwd=/", nullptr};

    kernelLog(LogLevel::DEBUG, "Creating first process.");
    Process *p = new Process();
    kernel::kernel.addProcess(*p);
    kernel::kernel.switchTask();
    if (kernel::kernel.exec("/bin/init", argv, envp))
    {
        kernelLog(LogLevel::PANIC, "Failed to load /bin/init");
        hacf();
    }

    kernelLog(LogLevel::INFO, "Bootup complete, loading first process...");
    load_context(kernel::kernel.getActiveProcess()->getContext());

    while (1)
        asm("nop");
}