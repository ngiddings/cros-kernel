#ifndef KERNEL_INTERRUPTS_H
#define KERNEL_INTERRUPTS_H

#include "interrupthandler.h"

namespace kernel::interrupt
{

    /**
     * @brief Static class which handles the enabling and disabling of interrupts,
     * as well as calling the appropriate handler for a particuler interrupt number.
     */
    class Interrupts
    {
    public:
        /**
         * @brief Initialize interrupts. Must be called before interrupts can work
         * properly. This function disables interrupts. When the caller is ready for
         * interrupts to fire, they must call `enable()`.
         */
        static void init();

        /**
         * @brief Enable interrupts.
         */
        static void enable();

        /**
         * @brief Disable interrupts.
         */
        static void disable();

        /**
         * @brief Insert a handler object to handle interrupts with the given id.
         * Only one handler may be associated with a given interrupt. If a handler
         * is already present for `id`, this function will overwrite it.
         * @param id
         * @param handler
         */
        static void insertHandler(int id, InterruptHandler *handler);

        /**
         * @param id ler associated with the given interrupt id.
         * @param id
         */
        static void callHandler(int id);

    private:
        /**
         * @brief Maximum size of the interrupt handler array. Must be at least as
         * big as the largest possible interrupt number.
         */
        static const int HANDLER_COUNT = 256;

        /**
         * @brief An array of pointers to handler objects. Each index in the array
         * corresponds to an interrupt number.
         */
        static InterruptHandler *handlers[HANDLER_COUNT];
    };

}

#endif