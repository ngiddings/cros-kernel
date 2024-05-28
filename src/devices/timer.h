#ifndef KERNEL_TIMER_H
#define KERNEL_TIMER_H

#include "irq/interrupthandler.h"

namespace kernel::devices 
{

class SystemTimer : public kernel::interrupt::InterruptHandler 
{
public:

    SystemTimer();

    SystemTimer(unsigned int delta);

    void handleInterrupt(int src);

private:

    enum TimerRegisters {
        CS = 0,
        CLO,
        CHI,
        C0,
        C1,
        C2,
        C3
    };

    static volatile unsigned int *const registers;

    unsigned int delta;

    void reset();
    
};

}  // namespace kernel::devices

#endif