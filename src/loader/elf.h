/**
 * Author: Nathan Giddings
 */
#ifndef KERNEL_ELF_H
#define KERNEL_ELF_H

#include "types/physaddr.h"
#include <cstdint>

namespace kernel::loader
{

    enum class ELFEndianness
    {
        LITTLE = 1,
        BIG = 2
    };

    enum class ELFISA
    {
        NA = 0x00,
        x86 = 0x03,
        MIPS = 0x08,
        PPC = 0x14,
        PPC64 = 0x15,
        ARM = 0x28,
        x86_64 = 0x3E,
        AARCH64 = 0xB7
    };

    enum class ELFSegmentType
    {
        UNUSED = 0,
        LOAD = 1,
        DYNAMIC = 2
    };

    class ELFFileHeader
    {
    public:
        uint32_t magic;
        char size;
        char endianness;
        char version;
        char abi;
        char abi_version;
        char reserved[7];
        uint16_t type;
        uint16_t machine;
        uint32_t _version;
        void *entry;
#if defined __i386__ || defined __arm__
        uint32_t phoffset;
        uint32_t shoffset;
#elif defined __x86_64__ || defined __aarch64__
        uint64_t phoffset;
        uint64_t shoffset;
#endif
        uint32_t flags;
        uint16_t header_size;
        uint16_t phsize;
        uint16_t phcount;
        uint16_t shsize;
        uint16_t shcount;
        uint16_t shstrndx;
    };

    class ELFProgramHeader
    {
    public:
        uint32_t type;
#if defined __i386__ || defined __arm__
        uint32_t offset;
        void *vaddr;
        physaddr_t paddr;
        uint32_t filesize;
        uint32_t memsize;
        uint32_t flags;
        uint32_t align;
#elif defined __x86_64__ || defined __aarch64__
        uint32_t flags;
        uint64_t offset;
        void *vaddr;
        physaddr_t paddr;
        uint64_t filesize;
        uint64_t memsize;
        uint64_t align;
#endif
    };

    class ELF
    {
    public:
        ELF(void *file);

        const void *fileLocation() const;

        bool isValid() const;

        const ELFFileHeader &fileHeader() const;

        const ELFProgramHeader &currentSection() const;

        bool nextSection();

    private:
        ELFFileHeader *header;

        int sectionIndex;
    };

    /**
     * @brief Creates a program image from the given binary in the current address space.
     * @param elf Binary to unpack
     * @return zero upon success, nonzero upon failure
     */
    int buildProgramImage(ELF &elf);

#if defined __i386__
    static const ELFISA HOST_ISA = ELFISA::x86;
#elif defined __x86_64__
    static const ELFISA HOST_ISA = ELFISA::x86_64;
#elif defined __arm__
    static const ELFISA HOST_ISA = ELFISA::ARM;
#elif defined __aarch64__
    static const ELFISA HOST_ISA = ELFISA::AARCH64;
#endif

}

#endif