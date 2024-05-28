#ifndef MAIN_H
#define MAIN_H

/*
 * Author: Hunter Overstake
 * Email: huntstake@gmail.com
 * Date: 5/14/2024
 */

#include "containers/string.h"
#include "containers/vector.h"

typedef unsigned char byte;

unsigned short bytes_to_short(const byte *bytes, int index);
unsigned int bytes_to_int(const byte *bytes, int index);
unsigned long bytes_to_long(const byte *bytes, int index);

void short_to_bytes(unsigned short num, byte *buffer, int offset);
void int_to_bytes(unsigned int num, byte *buffer, int offset);
void long_to_bytes(unsigned long num, byte *buffer, int offset);

wchar_t chars_to_unicode(unsigned char first, unsigned char second);
bool string_compare(string first, string second);

#endif