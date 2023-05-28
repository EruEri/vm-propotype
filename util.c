#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "util.h"


uint64_t alignn(uint64_t size, uint64_t to) {
    uint64_t div = size / to;
    uint64_t modulo = size % to == 0 ? 0 : 1;
    return to * (div + modulo);
}

uint64_t align8(uint64_t size) {
    return alignn(size, 8);
}

uint64_t bits_of_double(double d) {
    uint64_t* ptr = (uint64_t*) &d;
    return *ptr;
}

double double_of_bits(uint64_t t){
    double* ptr = (double*) &t;
    return *ptr;
}


void failwith(const char* message, int code) {
    puts(message);
    exit(code);
}
