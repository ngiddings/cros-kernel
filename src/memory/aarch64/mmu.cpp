#include "memory/mmap.h"
#include "memory/pageallocator.h"
#include "types/physaddr.h"
#include "aarch64/irq/exceptionclass.h"
#include "aarch64/irq/syndromedataabort.h"
#include "aarch64/sysreg.h"
#include "util/hacf.h"
#include "kernel.h"
#include "util/log.h"
#include <cstdint>

using namespace kernel::memory;

class PageTableEntry
{
public:
    /**
     * @brief Set when descriptor points to valid page or table
     */
    uint64_t present : 1;

    /**
     * @brief Descriptor type: 1 for table/page entry, 0 for block entry
     */
    uint64_t type : 1;

    /**
     * @brief Index into MAIR_ELn register
     */
    uint64_t attrIndex : 3;

    /**
     * @brief Non-secure
     */
    uint64_t ns : 1;

    /**
     * @brief Access permission[1]: when set, accessable by EL0
     *
     */
    uint64_t apEL0 : 1;

    /**
     * @brief Access permission[2]: when set, memory is read-only
     *
     */
    uint64_t apReadOnly : 1;

    /**
     * @brief Sharability
     */
    uint64_t sh : 2;

    /**
     * @brief Access flag
     */
    uint64_t af : 1;

    /**
     * @brief No global
     */
    uint64_t ng : 1;

    /**
     * @brief Significant bits of physical address of next table or page.
     */
    uint64_t outputAddress : 40;

    /**
     * @brief Privileged execute-never
     */
    uint64_t pxn : 1;

    /**
     * @brief Execute-never
     */
    uint64_t xn : 1;

    /**
     * @brief Copy-on-write
     */
    uint64_t cow : 1;

    /**
     * @brief Free for software use
     */
    uint64_t reserved : 3;

    /**
     * @brief When set, all child tables will be treated as if pxn=1
     */
    uint64_t pxnTable : 1;

    /**
     * @brief When set, all child tables will be treated as if xn=1
     */
    uint64_t xnTable : 1;

    /**
     * @brief When set, all child tables will be treated as if ap=1
     */
    uint64_t apTable : 2;

    /**
     * @brief when set, all child tables will be treated as if ns=1
     */
    uint64_t nsTable : 1;

    void clear()
    {
        *(uint64_t *)this = 0;
    }

    void makeTableDescriptor(physaddr_t nextLevel)
    {
        clear();
        present = 1;
        type = 1;
        af = 1;
        physicalAddress(nextLevel);
    }

    void makeBlockDescriptor(physaddr_t frame, int permissions)
    {
        clear();
        present = 1;
        apEL0 = (permissions & PAGE_USER) ? 1 : 0;
        apReadOnly = (permissions & PAGE_RW) ? 0 : 1;
        xn = (permissions & PAGE_EXE) ? 0 : 1;
        af = 1;
        physicalAddress(frame);
    }

    void makePageDescriptor(physaddr_t frame, int permissions)
    {
        clear();
        present = 1;
        type = 1;
        apEL0 = (permissions & PAGE_USER) ? 1 : 0;
        apReadOnly = (permissions & PAGE_RW) ? 0 : 1;
        xn = (permissions & PAGE_EXE) ? 0 : 1;
        af = 1;
        physicalAddress(frame);
    }

    void physicalAddress(physaddr_t addr)
    {
        outputAddress = addr >> 12;
    }

    physaddr_t physicalAddress()
    {
        return outputAddress << 12;
    }
};

PageTableEntry *const kernelTables[] = {
    (PageTableEntry *)0xFFFFFFFFFFFFF000,
    (PageTableEntry *)0xFFFFFFFFFFE00000,
    (PageTableEntry *)0xFFFFFFFFC0000000};

PageTableEntry *const userTables[] = {
    (PageTableEntry *)0x0000007FFFFFF000,
    (PageTableEntry *)0x0000007FFFE00000,
    (PageTableEntry *)0x0000007FC0000000};

const unsigned long kernel::memory::page_size = 4096;

void kernel::memory::loadAddressSpace(AddressSpace &addressSpace)
{
    unsigned long ttbr0 = addressSpace.getTableFrame() | ((unsigned long)addressSpace.getId() << 48);
    set_ttbr0_el1(ttbr0);
    asm volatile("DSB ISH");
    asm volatile("TLBI VMALLE1");
    asm volatile("DSB ISH");
    asm volatile("ISB");
}

void kernel::memory::initializeTopTable(physaddr_t frame)
{
    static PageTableEntry *scratchAddr = (PageTableEntry *)0xFFFFFFFFBFFFF000;
    physaddr_t buffer = getPageFrame(scratchAddr);
    setPageEntry(0, scratchAddr, frame, PAGE_RW);
    for (int i = 0; i < 511; i++)
    {
        ((unsigned long *)scratchAddr)[i] = 0;
    }
    scratchAddr[511].makeTableDescriptor(frame);
    setPageEntry(0, scratchAddr, buffer, PAGE_RW);
}

size_t kernel::memory::getBlockSize(int level)
{
    switch (level)
    {
    case 0:
        return (1UL << 12);
    case 1:
        return (1UL << 21);
    default:
        return 0;
    }
}

physaddr_t kernel::memory::getPageFrame(void *page)
{
    PageTableEntry *const *tables = (unsigned long)page > (unsigned long)&__high_mem ? kernelTables : userTables;
    unsigned long linearAddr = (unsigned long)page & 0x0000007FFFFFFFFF;

    for (int i = 0; i < 2; i++)
    {
        int index = linearAddr >> (30 - i * 9);
        if (!tables[i][index].present)
        {
            return 0;
        }
        else if (!tables[i][index].type)
        {
            return tables[i][index].physicalAddress();
        }
    }

    if (!tables[2][linearAddr >> 12].present)
    {
        return 0;
    }
    return tables[2][linearAddr >> 12].physicalAddress();
}

void kernel::memory::setPageEntry(int level, void *page, physaddr_t frame, int flags)
{
    if (level > 2)
    {
        return;
    }

    PageTableEntry *entry;
    if ((unsigned long)page >= (unsigned long)&__high_mem)
    {
        page = (void *)((unsigned long)page - (unsigned long)&__high_mem);
        unsigned long index = (unsigned long)page >> (12 + 9 * level);
        entry = &kernelTables[2 - level][index];
    }
    else if ((unsigned long)page < 0x0000008000000000)
    {
        unsigned long index = (unsigned long)page >> (12 + 9 * level);
        entry = &userTables[2 - level][index];
    }

    if (level == 0)
    {
        entry->makePageDescriptor(frame, flags);
    }
    else
    {
        entry->makeBlockDescriptor(frame, flags);
    }
    asm volatile("DSB ISH");
    asm volatile("TLBI VMALLE1");
    asm volatile("DSB ISH");
    asm volatile("ISB");
}

void kernel::memory::setTableEntry(int level, void *page, physaddr_t table)
{
    if (level == 0)
    {
        return;
    }

    PageTableEntry *entry;
    if ((unsigned long)page >= (unsigned long)&__high_mem)
    {
        page = (void *)((unsigned long)page - (unsigned long)&__high_mem);
        unsigned long index = (unsigned long)page >> (12 + 9 * level);
        entry = &kernelTables[2 - level][index];
    }
    else if ((unsigned long)page < 0x0000008000000000)
    {
        unsigned long index = (unsigned long)page >> (12 + 9 * level);
        entry = &userTables[2 - level][index];
    }

    entry->makeTableDescriptor(table);
    asm volatile("DSB ISH");
    asm volatile("TLBI VMALLE1");
    asm volatile("DSB ISH");
    asm volatile("ISB");
}

void kernel::memory::clearEntry(int level, void *page)
{
    PageTableEntry *entry;
    if ((unsigned long)page >= (unsigned long)&__high_mem)
    {
        page = (void *)((unsigned long)page - (unsigned long)&__high_mem);
        unsigned long index = (unsigned long)page >> (12 + 9 * level);
        entry = &kernelTables[2 - level][index];
    }
    else if ((unsigned long)page < 0x0000008000000000)
    {
        unsigned long index = (unsigned long)page >> (12 + 9 * level);
        entry = &userTables[2 - level][index];
    }
    entry->clear();
    asm volatile("DSB ISH");
    asm volatile("TLBI VMALLE1");
    asm volatile("DSB ISH");
    asm volatile("ISB");
}

void fillTranslationTable(int faultLevel, int targetLevel, void *far)
{
    // kernelLog(LogLevel::DEBUG, "fillTranslationTable(%i, %i, %016x)", faultLevel, targetLevel, far);
    PageTableEntry *const *tables = nullptr;
    if (far >= &__high_mem)
    {
        tables = kernelTables;
    }
    else
    {
        tables = userTables;
    }

    unsigned long farOffset = (unsigned long)far - (unsigned long)tables[targetLevel];
    for (int i = (targetLevel + faultLevel - 4); i < targetLevel; i++)
    {
        unsigned long tableIndex = farOffset >> (12 + 9 * (targetLevel - i - 1));
        physaddr_t tableFrame = pageAllocator.reserve(page_size);
        if (tableFrame == PageAllocator::NOMEM)
        {
            kernelLog(LogLevel::PANIC, "Out of memory while allocating page table");
            hacf();
        }
        tables[i][tableIndex].makeTableDescriptor(tableFrame);
        asm volatile("DSB ISH");
        asm volatile("TLBI VMALLE1");
        asm volatile("DSB ISH");
        asm volatile("ISB");
    }
}

void handlePageFault(ExceptionClass type, SyndromeDataAbort syndrome)
{
    void *far = get_far_el1();
    switch (syndrome.statusCode)
    {
    case DataAbortStatus::ACCESS_FAULT_0:
        kernelLog(LogLevel::PANIC, "Unhandled page fault (ACCESS_FAULT_0), FAR_EL1 = %016x", far);
        hacf();
        break;
    case DataAbortStatus::ACCESS_FAULT_1:
        kernelLog(LogLevel::PANIC, "Unhandled page fault (ACCESS_FAULT_1), FAR_EL1 = %016x", far);
        hacf();
        break;
    case DataAbortStatus::ACCESS_FAULT_2:
        kernelLog(LogLevel::PANIC, "Unhandled page fault (ACCESS_FAULT_2), FAR_EL1 = %016x", far);
        hacf();
        break;
    case DataAbortStatus::ACCESS_FAULT_3:
        kernelLog(LogLevel::PANIC, "Unhandled page fault (ACCESS_FAULT_3), FAR_EL1 = %016x", far);
        hacf();
        break;
    case DataAbortStatus::ADDR_SIZE_FAULT_0:
        kernelLog(LogLevel::PANIC, "Unhandled page fault (ADDR_SIZE_FAULT_0), FAR_EL1 = %016x", far);
        hacf();
        break;
    case DataAbortStatus::ADDR_SIZE_FAULT_1:
        kernelLog(LogLevel::PANIC, "Unhandled page fault (ADDR_SIZE_FAULT_1), FAR_EL1 = %016x", far);
        hacf();
        break;
    case DataAbortStatus::ADDR_SIZE_FAULT_2:
        kernelLog(LogLevel::PANIC, "Unhandled page fault (ADDR_SIZE_FAULT_2), FAR_EL1 = %016x", far);
        hacf();
        break;
    case DataAbortStatus::ADDR_SIZE_FAULT_3:
        kernelLog(LogLevel::PANIC, "Unhandled page fault (ADDR_SIZE_FAULT_3), FAR_EL1 = %016x", far);
        hacf();
        break;
    case DataAbortStatus::PERM_FAULT_0:
        kernelLog(LogLevel::PANIC, "Unhandled page fault (PERM_FAULT_0), FAR_EL1 = %016x", far);
        hacf();
        break;
    case DataAbortStatus::PERM_FAULT_1:
        kernelLog(LogLevel::PANIC, "Unhandled page fault (PERM_FAULT_1), FAR_EL1 = %016x", far);
        hacf();
        break;
    case DataAbortStatus::PERM_FAULT_2:
        kernelLog(LogLevel::PANIC, "Unhandled page fault (PERM_FAULT_2), FAR_EL1 = %016x", far);
        hacf();
        break;
    case DataAbortStatus::PERM_FAULT_3:
        kernelLog(LogLevel::PANIC, "Unhandled page fault (PERM_FAULT_3), FAR_EL1 = %016x", far);
        hacf();
        break;
    case DataAbortStatus::TRANSLATE_FAULT_0:
        kernelLog(LogLevel::PANIC, "Unhandled page fault (TRANSLATE_FAULT_0), FAR_EL1 = %016x", far);
        hacf();
        break;
    case DataAbortStatus::TRANSLATE_FAULT_1:
    case DataAbortStatus::TRANSLATE_FAULT_2:
    case DataAbortStatus::TRANSLATE_FAULT_3:
        int level = (int)syndrome.statusCode & 3;
        if (!syndrome.wnr)
        {
            kernelLog(LogLevel::PANIC, "Unhandled page fault (TRANSLATE_FAULT_%c), FAR_EL1 = %016x", '0' + level, get_far_el1());
            hacf();
        }

        if (far >= kernelTables[0])
        {
            kernelLog(LogLevel::PANIC, "Unhandled page fault (TRANSLATE_FAULT_%c), FAR_EL1 = %016x", '0' + level, get_far_el1());
            hacf();
        }
        else if (far >= kernelTables[1])
        {
            fillTranslationTable(level, 1, far);
        }
        else if (far >= kernelTables[2])
        {
            fillTranslationTable(level, 2, far);
        }
        else if (far >= userTables[0])
        {
            kernelLog(LogLevel::PANIC, "Unhandled page fault (TRANSLATE_FAULT_%c), FAR_EL1 = %016x", '0' + level, get_far_el1());
            hacf();
        }
        else if (far >= userTables[1])
        {
            fillTranslationTable(level, 1, far);
        }
        else if (far >= userTables[2])
        {
            fillTranslationTable(level, 2, far);
        }
        else if (far == nullptr)
        {
            kernelLog(LogLevel::PANIC, "Null pointer exception.");
            hacf();
        }
        else
        {
            kernelLog(LogLevel::PANIC, "Unhandled page fault (TRANSLATE_FAULT_%c), FAR_EL1 = %016x", '0' + level, get_far_el1());
            hacf();
        }
        break;
    }
}
