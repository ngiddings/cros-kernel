#ifndef KERNEL_SYNDROMEDATAABORT_H
#define KERNEL_SYNDROMEDATAABORT_H

#include <cstdint>

enum class DataAbortStatus
{
    ADDR_SIZE_FAULT_0 = 0x00,
    ADDR_SIZE_FAULT_1 = 0x01,
    ADDR_SIZE_FAULT_2 = 0x02,
    ADDR_SIZE_FAULT_3 = 0x03,
    TRANSLATE_FAULT_0 = 0x04,
    TRANSLATE_FAULT_1 = 0x05,
    TRANSLATE_FAULT_2 = 0x06,
    TRANSLATE_FAULT_3 = 0x07,
    ACCESS_FAULT_0 = 0x08,
    ACCESS_FAULT_1 = 0x09,
    ACCESS_FAULT_2 = 0x0A,
    ACCESS_FAULT_3 = 0x0B,
    PERM_FAULT_0 = 0x0C,
    PERM_FAULT_1 = 0x0D,
    PERM_FAULT_2 = 0x0E,
    PERM_FAULT_3 = 0x0F
};

struct SyndromeDataAbort
{
    /**
     * @brief Data fault status code
     */
    DataAbortStatus statusCode : 6;

    /**
     * @brief Write not read. Set when an abort is caused by an instruction
     * writing to a memory location. Clear when an abort is caused by a read.
     */
    uint64_t wnr : 1;

    /**
     * @brief 
     */
    uint64_t s1ptw : 1;

    /**
     * @brief Cache maintenance.
     */
    uint64_t cm : 1;

    /**
     * @brief External abort.
     */
    uint64_t ea : 1;

    /**
     * @brief FAR not valid for a synchronous external abort.
     */
    uint64_t fnv : 1;

    /**
     * @brief Synchronous error type.
     */
    uint64_t set : 2;

    /**
     * @brief Indicates that the fault came from use of VNCR_EL2 register.
     */
    uint64_t vncr : 1;

    /**
     * @brief Aquire/release.
     */
    uint64_t ar : 1;

    /**
     * @brief 64-bit GP register transfer.
     */
    uint64_t sf : 1;

    /**
     * @brief Register number of the faulting instruction.
     */
    uint64_t srt : 5;

    /**
     * @brief Syndrome sign extend.
     */
    uint64_t sse : 1;

    /**
     * @brief Syndrome access size.
     */
    uint64_t sas : 2;

    /**
     * @brief Instruction syndrome valid.
     */
    uint64_t isv : 1;
};

#endif