#ifndef UTIL_H
#define UTIL_H

#include <stdint.h>


typedef int bool_t;
#define true 1
#define false 0

uint64_t alignn(uint64_t size, uint64_t to);
uint64_t align8(uint64_t size);
uint64_t bits_of_double(double d);
double double_of_bits(uint64_t t);
void failwith(const char* message, int code);


#endif