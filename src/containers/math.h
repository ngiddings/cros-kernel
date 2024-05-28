#ifndef MATH_H
#define MATH_H

template <typename T>
const T& min(const T& x, const T& y) {
    return (x < y) ? x : y;
}

template <typename T>
const T& max(const T& x, const T& y) {
    return (x > y) ? x : y;
}

template <typename T>
const T abs(const T& x) {
    return (x < 0) ? -x : x;
}

template <typename T>
const T pow(const T& base, const T& exponent) {
    T result = 1;
    for (int i = 0; i < abs(exponent); i++) {
        result *= base;
    }
    return (exponent >= 0) ? result : 1 / result;
}

#endif