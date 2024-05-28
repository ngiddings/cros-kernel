#include "log.h"

#include <stddef.h>
#include <stdbool.h>

using namespace kernel;

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

static const char *FG_BLUE = "\x1b[34m";
static const char *FG_YELLOW = "\x1b[33m";
static const char *FG_RED = "\x1b[31m";
static const char *BOLD_RED = "\x1b[1;31m";
static const char *FG_FAINT = "\x1b[1;2m";
static const char *MODE_RESET = "\x1b[0m";

CharStream *stream = nullptr;

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

int logInit(CharStream *outStream)
{
    stream = outStream;
    return 0;
}

kernel::CharStream *getLogStream()
{
    return stream;
}

int vprintf(const char *format, va_list valist)
{
    if (stream == nullptr)
    {
        return 0;
    }

    while (*format)
    {
        if (*format == '%')
        {
            size_t width = 0;
            switch (*++format)
            {
            case FORMAT_PADDING:
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
                    *stream << '-';
                    n *= -1;
                }
                *stream << itoa((unsigned int)n, 10, width);
                break;
            }
            case FORMAT_UNSIGNED_DECIMAL:
                *stream << itoa(va_arg(valist, unsigned long), 10, width);
                break;
            case FORMAT_UNSIGNED_OCTAL:
                *stream << itoa(va_arg(valist, unsigned long), 8, width);
                break;
            case FORMAT_UNSIGNED_HEX:
                *stream << itoa(va_arg(valist, unsigned long), 16, width);
                break;
            case FORMAT_STRING:
                *stream << va_arg(valist, const char *);
                break;
            case FORMAT_CHARACTER:
                *stream << va_arg(valist, int);
                break;
            case FORMAT_PERCENT:
                *stream << '%';
                break;
            }
        }
        else
        {
            *stream << *format;
            if (*format == '\n')
            {
                *stream << '\r';
            }
        }
        format++;
    }
}

int printf(const char *format, ...)
{
    if (stream == nullptr)
    {
        return 0;
    }

    va_list valist;
    va_start(valist, format);
    vprintf(format, valist);
    va_end(valist);
    return 0;
}

void kernelLog(LogLevel level, const char *fmt, ...)
{
    va_list valist;
    va_start(valist, fmt);
    switch (level)
    {
#if !defined KERNEL_NODEBUG
    case LogLevel::DEBUG:
        printf("%s[Debug]: ", FG_FAINT);
        vprintf(fmt, valist);
        printf("%s\r\n", MODE_RESET);
        break;
#endif
    case LogLevel::INFO:
        printf("[%sInfo%s]: ", FG_BLUE, MODE_RESET);
        vprintf(fmt, valist);
        printf("\r\n");
        break;
    case LogLevel::WARNING:
        printf("[%sWarning%s]: ", FG_YELLOW, MODE_RESET);
        vprintf(fmt, valist);
        printf("\r\n");
        break;
    case LogLevel::ERROR:
        printf("[%sError%s]: ", FG_RED, MODE_RESET);
        vprintf(fmt, valist);
        printf("\r\n");
        break;
    case LogLevel::PANIC:
        printf("[%sPANIC%s]: ", BOLD_RED, MODE_RESET);
        vprintf(fmt, valist);
        printf("\r\n");
        break;
    default:
        break;
    }
    va_end(valist);
}
