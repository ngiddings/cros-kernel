/**
 * Author: Nathan Giddings
 * 
 * Source: https://stackoverflow.com/questions/11376288/fast-computing-of-log2-for-64-bit-integers
 */
#ifndef MATH_H
#define MATH_H

/**
 * @brief Quickly compute log2 of an int, rounding up.
 * @param x 
 * @return ceil(log2(x))
 */
static inline int ilog2(unsigned int x)
{
#if defined __GNUC__
    if(x <= 1)
        return 0;
    return 32 - __builtin_clz(x - 1);
#else
    static const int table[32] = {
        0,  9,  1, 10, 13, 21,  2, 29,
        11, 14, 16, 18, 22, 25,  3, 30,
        8, 12, 20, 28, 15, 17, 24,  7,
        19, 27, 23,  6, 26,  5,  4, 31};
    
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;

    return table[(x * 0x07C4ACDD) >> 27];
#endif
}

/**
 * @brief Quickly compute log2 of a long, rounding up.
 * @param x 
 * @return ceil(log2(x))
 */
static inline int llog2(unsigned long x)
{
#if (defined __GNUC__) && (__SIZEOF_LONG__ == 4)
    if(x <= 1)
        return 0;
    return 32 - __builtin_clzl(x - 1);
#elif (defined __GNUC__) && (__SIZEOF_LONG__ == 8)
    if(x <= 1)
        return 0;
    return 64 - __builtin_clzl(x - 1);
#elif __SIZEOF_LONG__ == 8
    static const int table[64] = {
        0, 58, 1, 59, 47, 53, 2, 60, 39, 48, 27, 54, 33, 42, 3, 61,
        51, 37, 40, 49, 18, 28, 20, 55, 30, 34, 11, 43, 14, 22, 4, 62,
        57, 46, 52, 38, 26, 32, 41, 50, 36, 17, 19, 29, 10, 13, 21, 56,
        45, 25, 31, 35, 16, 9, 12, 44, 24, 15, 8, 23, 7, 6, 5, 63};

    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    x |= x >> 32;

    return table[(x * 0x03f6eaf2cd271461) >> 58];
#else
    static const int table[32] = {
        0,  9,  1, 10, 13, 21,  2, 29,
        11, 14, 16, 18, 22, 25,  3, 30,
        8, 12, 20, 28, 15, 17, 24,  7,
        19, 27, 23,  6, 26,  5,  4, 31};
    
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;

    return table[(x * 0x07C4ACDD) >> 27];
#endif
}

#endif