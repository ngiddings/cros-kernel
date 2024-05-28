#ifndef _HACF_H
#define _HACF_H

extern "C"
/**
 * @brief Cease all meaningful execution. Implementation is platform-dependent.
 */
void hacf() __attribute__((noreturn));

#endif