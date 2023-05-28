#ifndef STACK_H
#define STACK_H

#include <stdint.h>
#include "util.h"

typedef struct {
    uint64_t* const memory;
    const uint64_t size;
    uint64_t sp;
} vm_stack_t;

vm_stack_t* stack_create(uint64_t size);
void free_stack(vm_stack_t* stack);
bool_t is_empty(vm_stack_t* stack);
bool_t push(vm_stack_t* stack, uint64_t value);
uint64_t pop(vm_stack_t* stack);

#endif