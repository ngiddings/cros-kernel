#include "uart.h"
#include "mmio.h"
#include "util/log.h"
#include "util/string.h"
#include <stdint.h>

namespace kernel::devices
{

    enum UARTRegisters
    {
        /**
         * @brief Data Register
         */
        DR = 0,

        /**
         * @brief Receive status register/error clear register
         */
        RSRECR = 1,

        /**
         * @brief Flag register
         */
        FR = 6,

        /**
         * @brief Not in use
         */
        ILPR = 8,

        /**
         * @brief Integer baud rate divisor
         */
        IBRD = 9,

        /**
         * @brief Fractional baud rate divisor
         */
        FBRD = 10,

        /**
         * @brief Line control register
         */
        LCRH = 11,

        /**
         * @brief Control register
         */
        CR = 12,

        /**
         * @brief Interrupt FIFO level select register
         */
        IFLS = 13,

        /**
         * @brief Interrupt mask set/clear register
         */
        IMSC = 14,

        /**
         * @brief Raw interrupt status register
         */
        RIS = 15,

        /**
         * @brief Masked interrupt status register
         */
        MIS = 16,

        /**
         * @brief Interrupt clear register
         */
        ICR = 17,

        /**
         * @brief DMA control register
         */
        DMACR = 18,

        /**
         * @brief Test control register
         */
        ITCR = 32,

        /**
         * @brief Integration test input register
         */
        ITIP = 33,

        /**
         * @brief Integration test output register
         */
        ITOP = 34,

        /**
         * @brief Test data register
         */
        IDR = 35

    };

    volatile unsigned int __attribute__((aligned(16))) mbox[9] = {
        9 * 4, 0, 0x38002, 12, 8, 2, 3000000, 0, 0};

    // Loop <delay> times in a way that the compiler won't optimize away
    static inline void delay(int32_t count)
    {
        /*asm volatile("__delay_%=: subs %[count], %[count], #1; bne __delay_%=\n"
                     : "=r"(count) : [count] "0"(count) : "cc");*/
        while (count > 0)
        {
            count--;
        }
    }

    UART::UART()
        : registers(nullptr), bufferIndex(0)
    {
        memset(buffer, bufferSize, 0);
    }

    UART::UART(void *mmio_offset)
        : registers((uint32_t *)mmio_offset), bufferIndex(0)
    {
        memset(buffer, bufferSize, 0);
        /*int raspi = 3;
        // Disable UART0.
        registers[CR] = 0;
        // Setup the GPIO pin 14 && 15.

        // Disable pull up/down for all GPIO pins & delay for 150 cycles.
        mmio_write((void *)MMIOOffset::GPPUD, 0x00000000);
        delay(150);

        // Disable pull up/down for pin 14,15 & delay for 150 cycles.
        mmio_write((void *)MMIOOffset::GPPUDCLK0, (1 << 14) | (1 << 15));
        delay(150);

        // Write 0 to GPPUDCLK0 to make it take effect.
        mmio_write((void *)MMIOOffset::GPPUDCLK0, 0x00000000);

        // Clear pending interrupts.
        registers[ICR] = 0x7FF;

        // Set integer & fractional part of baud rate.
        // Divider = UART_CLOCK/(16 * Baud)
        // Fraction part register = (Fractional part * 64) + 0.5
        // Baud = 115200.

        // For Raspi3 and 4 the UART_CLOCK is system-clock dependent by default.
        // Set it to 3Mhz so that we can consistently set the baud rate
        if (raspi >= 3)
        {
            // UART_CLOCK = 30000000;
            unsigned int r = (((unsigned int)(&mbox) & ~0xF) | 8);
            // wait until we can talk to the VC
            while (mmio_read((void *)MMIOOffset::MBOX_STATUS) & 0x80000000)
            {
            }
            // send our message to property channel and wait for the response
            mmio_write((void *)MMIOOffset::MBOX_WRITE, r);
            while ((mmio_read((void *)MMIOOffset::MBOX_STATUS) & 0x40000000) || mmio_read((void *)MMIOOffset::MBOX_READ) != r)
            {
            }
        }

        // Divider = 3000000 / (16 * 115200) = 1.627 = ~1.
        registers[IBRD] = 1;
        // Fractional part register = (.627 * 64) + 0.5 = 40.6 = ~40.
        registers[FBRD] = 40;

        // Enable FIFO & 8 bit data transmission (1 stop bit, no parity).
        registers[LCRH] = (1 << 4) | (1 << 5) | (1 << 6);

        // Mask all interrupts.
        */
        registers[IMSC] = Receive | /*Transmit |*/ ReceiveTimeout |
                          Framing | Parity | Break | Overrun
            /*0*/
            ;

        // Enable UART0, receive & transfer part of UART.
        // registers[CR] = (1 << 0) | (1 << 8) | (1 << 9);
    }

    UART &UART::operator<<(char c)
    {
        while (registers[UARTRegisters::FR] & (1 << 5))
            ;
        registers[UARTRegisters::DR] = c;
        return *this;
    }

    UART &UART::operator<<(const char *str)
    {
        for (const char *s = str; *s != '\0'; s++)
        {
            *this << *s;
        }
        return *this;
    }

    UART &UART::operator>>(char &c)
    {
        while (registers[FR] & (1 << 4))
        {
        }
        c = registers[DR];
    }

    kernel::fs::FileContext *UART::open(Mode mode)
    {
        return new UARTContext(*this);
    }

    void UART::close(int id)
    {
    }

    void UART::handleInterrupt(int src)
    {
        int status = registers[UARTRegisters::MIS];
        if (status & Receive)
        {
            // kernelLog(LogLevel::DEBUG, "UART receive interrupt");
            readByte();
        }
        if (status & Transmit)
        {
            // kernelLog(LogLevel::DEBUG, "UART transmit interrupt");
        }
        if (status & CTSM)
        {
            kernelLog(LogLevel::DEBUG, "UART nUARTCTS modem interrupt");
        }
        if (status & ReceiveTimeout)
        {
            // kernelLog(LogLevel::DEBUG, "UART receive timeout interrupt");
            readByte();
        }
        if (status & Framing)
        {
            kernelLog(LogLevel::DEBUG, "UART framing interrupt");
        }
        if (status & Parity)
        {
            kernelLog(LogLevel::DEBUG, "UART parity interrupt");
        }
        if (status & Break)
        {
            kernelLog(LogLevel::DEBUG, "UART break interrupt");
        }
        if (status & Overrun)
        {
            kernelLog(LogLevel::DEBUG, "UART overrun interrupt");
        }
        registers[UARTRegisters::ICR] = /*0x7F2*/ status;
    }

    void UART::readByte()
    {
        while (!(registers[UARTRegisters::FR] & (1 << 4)))
        {
            buffer[bufferIndex] = registers[UARTRegisters::DR];
            bufferIndex++;
            if (bufferIndex >= bufferSize)
            {
                bufferIndex = 0;
            }
        }
    }
}

kernel::devices::UART::UARTContext::UARTContext(UART &uart)
    : uart(uart), pos(uart.bufferIndex)
{
}

kernel::devices::UART::UARTContext::~UARTContext()
{
}

int kernel::devices::UART::UARTContext::read(void *buffer, int n)
{
    char *s = (char *)buffer;
    int c = 0;
    while (pos != uart.bufferIndex && c < n)
    {
        s[c] = uart.buffer[pos];
        c++;
        pos++;
        if (pos >= uart.bufferSize)
        {
            pos = 0;
        }
    }
    return c;
}

int kernel::devices::UART::UARTContext::write(const void *buffer, int n)
{
    for (int i = 0; i < n; i++)
    {
        uart << ((char *)buffer)[i];
        if (((char *)buffer)[i] == '\n')
        {
            uart << '\r';
        }
    }
    return n;
}

kernel::fs::FileContext *kernel::devices::UART::UARTContext::copy()
{
    UARTContext *f = new UARTContext(uart);
    if (f != nullptr)
    {
        f->pos = pos;
    }
    return f;
}
