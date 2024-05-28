#ifndef KERNEL_MMIO_H
#define KERNEL_MMIO_H

#include <cstdint>

enum class MMIOOffset : uint64_t
{
    /**
     * @brief Base address for GPIO registers
     */
    GPIO_BASE = 0x200000,

    // Controls actuation of pull up/down to ALL GPIO pins.
    GPPUD = (GPIO_BASE + 0x94),

    // Controls actuation of pull up/down for specific GPIO pin.
    GPPUDCLK0 = (GPIO_BASE + 0x98),

    /**
     * @brief Base address for UART0 registers.
     */
    UART0_BASE = (GPIO_BASE + 0x1000), // for raspi4 0xFE201000, raspi2 & 3 0x3F201000, and 0x20201000 for raspi1

    INTR_OFFSET = 0xB000,
    INTR_BASIC_PENDING = INTR_OFFSET + 0x200,
    INTR_IRQ_PENDING_1 = INTR_OFFSET + 0x204,
    INTR_IRQ_PENDING_2 = INTR_OFFSET + 0x208,
    INTR_FIQ_CTRL = INTR_OFFSET + 0x20C,
    INTR_IRQ_ENABLE_1 = INTR_OFFSET + 0x210,
    INTR_IRQ_ENABLE_2 = INTR_OFFSET + 0x214,
    INTR_IRQ_ENABLE_BASE = INTR_OFFSET + 0x218,
    INTR_IRQ_DISABLE_BASE = INTR_OFFSET + 0x224,
    INTR_IRQ_DISABLE_1 = INTR_OFFSET + 0x21C,
    INTR_IRQ_DISABLE_2 = INTR_OFFSET + 0x220,

    MBOX_BASE = 0xB880,
    MBOX_READ = (MBOX_BASE + 0x00),
    MBOX_STATUS = (MBOX_BASE + 0x18),
    MBOX_WRITE = (MBOX_BASE + 0x20)
};

/**
 * @brief Write to a 32-bit MMIO register
 *
 * @param reg pointer to the register to write to
 * @param data the data to write to the specified register
 */
static inline void mmio_write(void *reg, uint32_t data)
{
    *(volatile uint32_t *)(reg + 0xFFFFFF803F000000) = data;
}

/**
 * @brief Read from a 32-bit MMIO register
 *
 * @param reg pointer to the register to read from
 * @return uint32_t the data read from the specified register
 */
static inline uint32_t mmio_read(void *reg)
{
    return *(volatile uint32_t *)(reg + 0xFFFFFF803F000000);
}

#endif