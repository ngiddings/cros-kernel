#include "timer.h"
#include "kernel.h"
#include "util/log.h"

volatile unsigned int *const kernel::devices::SystemTimer::registers = (unsigned int *)0xFFFFFF803F003000;

kernel::devices::SystemTimer::SystemTimer()
{
    this->delta = 50;
    reset();
}

kernel::devices::SystemTimer::SystemTimer(unsigned int delta)
{
    this->delta = delta;
    reset();
}

void kernel::devices::SystemTimer::handleInterrupt(int src)
{
    reset();
    // kernelLog(LogLevel::DEBUG, "Timer interrupt: %i", registers[CLO]);
    kernel::kernel.switchTask();
}

void kernel::devices::SystemTimer::reset()
{
    unsigned int currentTime = registers[CLO];
    registers[C1] = currentTime + (delta * 1000);
    registers[CS] = 0xF;
}
