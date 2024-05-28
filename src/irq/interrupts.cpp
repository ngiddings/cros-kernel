#include "interrupts.h"

kernel::interrupt::InterruptHandler *kernel::interrupt::Interrupts::handlers[256];

void kernel::interrupt::Interrupts::insertHandler(int id, InterruptHandler *handler)
{
    if(id < HANDLER_COUNT)
    {
        handlers[id] = handler;
    }
}

void kernel::interrupt::Interrupts::callHandler(int id)
{
    if(handlers[id] != nullptr)
    {
        handlers[id]->handleInterrupt(id);
    }
}