#include <cstdint>
#include <cstddef>
#include <cstdarg>

#define BASE 0x3f000000

#define MBOX_READ ((volatile unsigned int *)(BASE + 0xB880 + 0x00))
#define MBOX_STATUS ((volatile unsigned int *)(BASE + 0xB880 + 0x18))
#define MBOX_WRITE ((volatile unsigned int *)(BASE + 0xB880 + 0x20))

#define GPPUD ((volatile unsigned int *)(BASE + 0x200000 + 0x94))
#define GPPUDCLK0 ((volatile unsigned int *)(BASE + 0x200000 + 0x98))

#define UART_DR ((volatile unsigned int *)(BASE + 0x200000 + 0x1000 + 0x00))
#define UART_FR ((volatile unsigned int *)(BASE + 0x200000 + 0x1000 + 0x18))
#define UART_IBRD ((volatile unsigned int *)(BASE + 0x200000 + 0x1000 + 0x24))
#define UART_FBRD ((volatile unsigned int *)(BASE + 0x200000 + 0x1000 + 0x28))
#define UART_LCRH ((volatile unsigned int *)(BASE + 0x200000 + 0x1000 + 0x2C))
#define UART_CR ((volatile unsigned int *)(BASE + 0x200000 + 0x1000 + 0x30))
#define UART_IMSC ((volatile unsigned int *)(BASE + 0x200000 + 0x1000 + 0x38))
#define UART_ICR ((volatile unsigned int *)(BASE + 0x200000 + 0x1000 + 0x44))

#define TIME_CLO ((volatile unsigned int *)(BASE + 0x3004))
#define TIME_CHI ((volatile unsigned int *)(BASE + 0x3008))

enum format_flags_t
{
    FORMAT_PADDING = '0',
    FORMAT_WIDTH = '*',

    FORMAT_SIGNED_DECIMAL = 'i',
    FORMAT_UNSIGNED_DECIMAL = 'u',
    FORMAT_UNSIGNED_OCTAL = 'o',
    FORMAT_UNSIGNED_HEX = 'x',
    FORMAT_STRING = 's',
    FORMAT_CHARACTER = 'c',
    FORMAT_COUNT = 'n',
    FORMAT_PERCENT = '%'

};

// Loop <delay> times in a way that the compiler won't optimize away
static inline void delay(int32_t count)
{
    while (count > 0)
    {
        count--;
    }
}

static char *itoa(unsigned long n, unsigned int base, unsigned int width)
{
    if (base < 2 || base > 16)
    {
        return NULL;
    }
    static const char *digits = "0123456789abcdef";
    static char buffer[65];
    char *s = &buffer[64];
    *s = 0;
    unsigned int count = 0;
    do
    {
        *--s = digits[n % base];
        n /= base;
        count++;
    } while (count < width || n != 0);
    return s;
}

// A Mailbox message with set clock rate of PL011 to 3MHz tag
volatile unsigned int __attribute__((aligned(16))) mbox[9] = {
    9 * 4, 0, 0x38002, 12, 8, 2, 3000000, 0, 0};

static void uart_init()
{
    *UART_CR = 0;
    *GPPUD = 0;
    delay(150);

    *GPPUDCLK0 = (1 << 14) | (1 << 15);
    delay(150);

    *GPPUDCLK0 = 0;
    *UART_ICR = 0x7FF;

    unsigned int r = (((unsigned int)(&mbox) & ~0xF) | 8);
    while (*MBOX_STATUS & 0x80000000)
    {
    }

    *MBOX_WRITE = r;
    while ((*MBOX_STATUS & 0x40000000) || *MBOX_READ != r)
    {
    }

    *UART_IBRD = 1;
    *UART_FBRD = 40;
    *UART_LCRH = (1 << 4) | (1 << 5) | (1 << 6);
    //*UART_IMSC = 0; //(1 << 1) | (1 << 4) | (1 << 5) | (1 << 6) | (1 << 7) | (1 << 8) | (1 << 9) | (1 << 10);
    *UART_CR = (1 << 0) | (1 << 8) | (1 << 9);
}

static void uart_puts(const char *str)
{
    for (const char *s = str; *s != '\0'; s++)
    {
        while (*UART_FR & (1 << 5))
        {
        }
        *UART_DR = *s;
    }
}

static void uart_putc(unsigned int c)
{
    while (*UART_FR & (1 << 5))
    {
    }
    *UART_DR = c;
}

static unsigned long get_time()
{
    return ((unsigned long)*TIME_CHI << 32UL) + (unsigned long)*TIME_CLO;
}

static int vprintf(const char *format, va_list valist)
{
    while (*format)
    {
        if (*format == '%')
        {
            size_t width = 0;
            bool padding = false;
            switch (*++format)
            {
            case FORMAT_PADDING:
                padding = true;
                format++;
                break;
            }
            while (*format >= '0' && *format <= '9')
            {
                width = (width * 10) + *format - '0';
                format++;
            }
            switch (*format)
            {
            case FORMAT_SIGNED_DECIMAL:
            {
                int n = va_arg(valist, int);
                if (n < 0)
                {
                    uart_puts("-");
                    n *= -1;
                }
                uart_puts(itoa((unsigned int)n, 10, width));
                break;
            }
            case FORMAT_UNSIGNED_DECIMAL:
                uart_puts(itoa(va_arg(valist, unsigned int), 10, width));
                break;
            case FORMAT_UNSIGNED_OCTAL:
                uart_puts(itoa(va_arg(valist, unsigned int), 8, width));
                break;
            case FORMAT_UNSIGNED_HEX:
                uart_puts(itoa(va_arg(valist, unsigned long), 16, width));
                break;
            case FORMAT_STRING:
                uart_puts(va_arg(valist, const char *));
                break;
            case FORMAT_CHARACTER:
                uart_putc(va_arg(valist, unsigned int));
                break;
            case FORMAT_PERCENT:
                uart_putc('%');
                break;
            }
        }
        else
        {
            uart_putc(*format);
        }
        format++;
    }
    return 0;
}

static int printf(const char *format, ...)
{
    va_list valist;
    va_start(valist, format);
    vprintf(format, valist);
    va_end(valist);
    return 0;
}

static void log(const char *fmt, ...)
{
    unsigned long time = get_time();
    printf("[bootstrap][%i:%03i:%03i]: ", time / 1000000, (time / 1000) % 1000, time % 1000);
    va_list valist;
    va_start(valist, fmt);
    vprintf(fmt, valist);
    uart_puts("\r\n");
    va_end(valist);
}

extern "C" void handle_ex(uint64_t syndrome)
{
    uint64_t far;
    asm volatile("mrs %0, far_el1" : "=r"(far));
    log("Fatal error during bootstrap:\r\n\tESR_EL1 = %016x\r\n\tFAR_EL1 = %016x", syndrome, far);
    while (true)
    {
    }
}

extern "C" void mmu_init(uint64_t kernel_size, uint64_t *level0, uint64_t *level1, uint64_t *level2)
{
    uart_init();
    log("Bootstrap start:\r\n\tkernel_size: %i MiB\r\n\tlevel0: %08x\r\n\tlevel1: %08x\r\n\tlevel2: %08x", kernel_size >> 20, level0, level1, level2);
    void *linearAddress = (void *)0;
    unsigned long offset = (unsigned long)linearAddress & 0x0000FFFFFFFFFFFF;
    int level0Index = offset >> 39;
    int level1Index = offset >> 30;
    int level2Index = offset >> 21;

    /*
     * Point level 0 entry at level 1 entry; level 1 entry at level 2 entry,
     * level 2 entry at physical address 0. Maps first 2MiB in kernel address
     * space to first 2 MiB of physical addresses.
     */
    level0[level0Index] = (uint64_t)level1 | 1027;
    level1[level1Index] = (uint64_t)level2 | 1027;
    for (unsigned long p = 0; p < kernel_size; p += 0x200000)
    {
        level2[level2Index + (p / 0x200000)] = p | 1025;
    }
    level2[504] = 0x3f000000 | 1025 | 4;
    level2[505] = 0x3f200000 | 1025 | 4;

    log("Filled kernel table entries.");

    /*
     * Point last entry of level 0 table at level 0 table.
     * Maps all tage tables into virtual address space, greatly simplifying
     * process of manipulating tables.
     */
    level1[511] = (uint64_t)level1 | 1027;

    log("Filled loopback entry.");

    /*
     * Point both translation table base registers to level0
     * (allows creation of temporary identity map)
     */
    asm volatile("msr TTBR0_EL1, %0" ::"r"(level1 + 1));
    asm volatile("msr TTBR1_EL1, %0" ::"r"(level1 + 1));

    log("Set TTBR registers.");

    uint64_t mmfr0;
    asm volatile("mrs %0, id_aa64mmfr0_el1" : "=r"(mmfr0));

    log("ID_AA64MMFR0 = %016x", mmfr0);

    unsigned long mair = (0xFF << 0) | // AttrIdx=0: normal, IWBWA, OWBWA, NTR
                         (0x04 << 8) | // AttrIdx=1: device, nGnRE (must be OSH too)
                         (0x44 << 16); // AttrIdx=2: non cacheable
    asm volatile("msr MAIR_EL1, %0" : : "r"(mair));

    log("Set MAIR_EL1 = %016x", mair);

    /*
     * Configure the translation control register:
     * - 4KiB granules for both translation tables
     * - 48-bit intermediate physical address size
     * - 16 most significant bits select either TTBR0 or TTBR1
     */
    uint64_t tcr;
    asm volatile("mrs %0, TCR_EL1" : "=r"(tcr));
    tcr = (0b00LL << 37) |        // TBI=0, no tagging
          ((mmfr0 & 0xF) << 32) | // IPS=autodetected
          (0b10LL << 30) |        // TG1=4k
          (0b11LL << 28) |        // SH1=3 inner
          (0b01LL << 26) |        // ORGN1=1 write back
          (0b01LL << 24) |        // IRGN1=1 write back
          (0b0LL << 23) |         // EPD1 enable higher half
          (25LL << 16) |          // T1SZ=25, 3 levels (512G)
          (0b00LL << 14) |        // TG0=4k
          (0b11LL << 12) |        // SH0=3 inner
          (0b01LL << 10) |        // ORGN0=1 write back
          (0b01LL << 8) |         // IRGN0=1 write back
          (0b0LL << 7) |          // EPD0 enable lower half
          (25LL << 0);            // T0SZ=25, 3 levels (512G)
    asm volatile("msr TCR_EL1, %0" ::"r"(tcr));
    asm volatile("isb");

    log("Set TCR_EL1 = %016x", tcr);

    /*
     * Enable MMU by setting bit 0 of SCTLR
     */
    uint64_t sctlr;
    asm volatile("dsb ish");
    asm volatile("isb");
    asm volatile("mrs %0, SCTLR_EL1" : "=r"(sctlr));
    sctlr |= 0xC00801;
    sctlr &= ~((1 << 25) | // clear EE, little endian translation tables
               (1 << 24) | // clear E0E
               (1 << 19) | // clear WXN
               (1 << 12) | // clear I, no instruction cache
               (1 << 4) |  // clear SA0
               (1 << 3) |  // clear SA
               (1 << 2) |  // clear C, no cache at all
               (1 << 1));  // clear A, no aligment check
    asm volatile("msr SCTLR_EL1, %0" ::"r"(sctlr));
    asm volatile("isb");

    log("Set SCTLR_EL1 = %016x", sctlr);
    log("Enabled MMU.");
}