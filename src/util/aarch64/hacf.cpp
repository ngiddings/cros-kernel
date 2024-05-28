#include "../hacf.h"
#include <stdint.h>

extern "C" void hacf()
{
    uint64_t daif = 0xF << 6;
    asm("msr daif, %0" ::"r"(daif));
    while (1)
    {
        asm("wfi");
    }
}