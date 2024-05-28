#ifndef KERNEL_INTERRUPTHANDLER_H
#define KERNEL_INTERRUPTHANDLER_H

namespace kernel::interrupt
{

    /**
     * @brief An interface which drivers that wish to handle hardware interrupts
     * must implement. The handleInterrupt() method will be called when the associated
     * interrupt number fires.
     */
    class InterruptHandler
    {
    public:
        /**
         * @brief Method to be called when the interrupt number associated with this
         * handler fires. `source` is the interrupt number which caused the handler
         * to be called. A handler object may choose to handle multiple interrupt
         * numbers.
         * @param source
         */
        virtual void handleInterrupt(int source) = 0;
    };

}

#endif