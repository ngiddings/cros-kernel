#ifndef _CHARSTREAM_H
#define _CHARSTREAM_H

#include "fs/filecontext.h"

namespace kernel
{

    /**
     * @brief Abstract class representing an object that accepts character data
     * for output.
     */
    class CharStream
    {
    public:
        enum class Mode
        {
            RO,
            RW,
            W
        };

        /**
         * @brief Outputs a single character
         * @param c The character to output
         * @return A reference to this object
         */
        virtual CharStream &operator<<(char c) = 0;

        /**
         * @brief Outputs an entire null-terminated string.
         * @param str The string to output
         * @return A reference to this object
         */
        virtual CharStream &operator<<(const char *str) = 0;

        virtual CharStream &operator>>(char &c) = 0;

        virtual kernel::fs::FileContext *open(Mode mode) = 0;

        virtual void close(int id) = 0;
    };

}

#endif