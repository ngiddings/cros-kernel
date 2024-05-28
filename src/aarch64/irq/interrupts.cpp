#include "irq/interrupts.h"
#include "exceptions.h"
#include "aarch64/sysreg.h"
#include "devices/mmio.h"

void kernel::interrupt::Interrupts::init()
{
    set_vbar_el1(&vector_table_el1);
    disable();
    for (int i = 0; i < HANDLER_COUNT; i++)
    {
        handlers[i] = nullptr;
    }
    mmio_write((void *)MMIOOffset::INTR_IRQ_ENABLE_BASE, 0x80002);
    // mmio_write((void *)MMIOOffset::INTR_IRQ_DISABLE_BASE, 0xFE);
    mmio_write((void *)MMIOOffset::INTR_IRQ_ENABLE_1, 0x00000002 /*0xFFFFFFFF*/);
    // mmio_write((void *)MMIOOffset::INTR_IRQ_DISABLE_1, 0x20000000);
    mmio_write((void *)MMIOOffset::INTR_IRQ_ENABLE_2, 0x2000000);
    // mmio_write((void *)MMIOOffset::INTR_IRQ_DISABLE_2, 0xFF6800);
}

void kernel::interrupt::Interrupts::enable()
{
    set_daif(0);
}

void kernel::interrupt::Interrupts::disable()
{
    set_daif(15 << 6);
}