#ifndef KERNEL_SIGNALACTION_H
#define KERNEL_SIGNALACTION_H

#include "actiontype.h"

namespace kernel::sched
{

    struct SignalAction
    {
        ActionType type;
        void (*handler)(void *);
        void (*trampoline)(void);
        void *userdata;
    };

}

#endif