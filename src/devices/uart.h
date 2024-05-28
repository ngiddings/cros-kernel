#ifndef KERNEL_UART_H
#define KERNEL_UART_H

#include "util/charstream.h"
#include "irq/interrupthandler.h"
#include "containers/binary_search_tree.h"
#include "fs/filecontext.h"
#include <stdint.h>

namespace kernel::devices
{

    class UART : public CharStream, public kernel::interrupt::InterruptHandler
    {
    public:
        UART();

        UART(void *mmio_offset);

        UART &operator<<(char c);

        UART &operator<<(const char *str);

        UART &operator>>(char &c);

        kernel::fs::FileContext *open(Mode mode);

        void close(int id);

        void handleInterrupt(int src);

    private:
        enum InterruptBits
        {
            CTSM = (1 << 1),
            Receive = (1 << 4),
            Transmit = (1 << 5),
            ReceiveTimeout = (1 << 6),
            Framing = (1 << 7),
            Parity = (1 << 8),
            Break = (1 << 9),
            Overrun = (1 << 10)
        };

        class UARTContext : public kernel::fs::FileContext
        {
        public:
            UARTContext(UART &uart);

            ~UARTContext();

            int read(void *buffer, int n);

            int write(const void *buffer, int n);

            kernel::fs::FileContext *copy();

        private:
            UART &uart;

            int pos;
        };

        static const int bufferSize = 4096;

        uint32_t *volatile registers;

        char buffer[bufferSize];

        int bufferIndex;

        void readByte();
    };

}

#endif