#ifndef KERNEL_ACTIONTYPE_H
#define KERNEL_ACTIONTYPE_H

namespace kernel::sched
{
    enum class ActionType
    {
        NONE,
        HANDLER,
        KILL
    };
}

#endif