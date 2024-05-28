#ifndef LOG_H
#define LOG_H

#include "charstream.h"
#include <stdarg.h>

enum class LogLevel
{
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    PANIC
};

/**
 * @brief Initialize the logger to output characters to `outStream`.
 * @param outStream
 * @return Zero upon success, nonzero upon failure.
 */
int logInit(kernel::CharStream *outStream);

/**
 * @return the stream being used for the kernel log
 */
kernel::CharStream *getLogStream();

/**
 * @brief C-style printf function, accepting commonly used formatting flags.
 * Outputs to the stream provided to `logInit`. Calls to this function before
 * a call to `logInit` will result in no data being outputted.
 *
 * @param format
 * @param
 * @return zero
 */
int printf(const char *format, ...);

/**
 * @brief Logs a formatted message tagged with the log level. Appends a newline
 * and carriage return after each message, so log messages need not include
 * them.
 *
 * @param level The log level of the message
 * @param fmt Format string of the log message
 * @param
 */
void kernelLog(LogLevel level, const char *fmt, ...);

#endif
