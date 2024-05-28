#ifndef KERNEL_SIGNALTYPE_H
#define KERNEL_SIGNALTYPE_H

namespace kernel::sched
{

    enum class SignalType
    {
        Unknown = 0,
        Hangup,
        Interrupt,
        Quit,
        IllegalInstruction,
        Trap,
        Abort,
        Bus,
        FloatingPoint,
        Kill,
        User_1,
        Segfault,
        User_2,
        Pipe,
        Alarm,
        Terminate,
        StackFault,
        Child,
        Continue,
        Stop
    };

}

#endif