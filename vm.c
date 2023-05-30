#include "vm.h"
#include "stack.h"
#include "util.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HALT_BITS 0x0
#define RET_BITS 0x01
#define SYSCALL_BITS 0x2
#define CALL_BITS 0x3

#define opcode_value(instruction) \
    (((uint32_t) instruction & OPCODE_MASK) >> (INSTRUCTION_SIZE - OPCODE_SIZE))

#define is_set(instruction, mask) \
    ((instruction & mask) == mask)

vm_t* vm_init(const instruction_t *const code, uint64_t stack_size, uint64_t offset) {
    vm_t* vm_ptr = malloc(sizeof(vm_t));
    if (!vm_ptr) failwith("Vm alloc fail", 1);
    vm_stack_t* stack = stack_create(stack_size);
    const instruction_t* ip = code + offset;
    vm_t vm = {.stack = stack, .code = code, .ip = ip, .fp = stack->sp};
    memcpy(vm_ptr, &vm, sizeof(vm_t));
    return vm_ptr;
}

instruction_t fetch_instruction(vm_t* vm) {
    return *(vm->ip++);
}

reg_t* register_of_int32(vm_t* vm, uint32_t bits, uint32_t shift) {
    switch (bits >> shift) {
    case 0:
        return &vm->r0;
    case 1:
        return &vm->r1;
    case 2:
        return &vm->r2;
    case 3:
        return &vm->r3; 
    case 4:
        return &vm->r4; 
    case 5:
        return &vm->r5; 
    case 6:
        return &vm->r6; 
    case 7:
        return &vm->r7; 
    case 8:
        return &vm->fr0; 
    case 9:
        return &vm->fr1; 
    case 10:
        return &vm->fr2; 
    case 11:
        return &vm->fr3; 
    case 12:
        return &vm->fr4; 
    case 13:
        return &vm->fr5; 
    case 14:
        return &vm->fr6; 
    case 15:
        return &vm->fr7; 
    default:
        failwith("Wrong register number", 1);
    }
    return (void *) 0;
}

int halt_opcode(vm_t* vm, instruction_t i) {
    switch ((i >> 28) & 0x3) {
        case HALT:{
            return 0;
        }
        case RET_BITS: {
            return 0;
        }

        case SYSCALL_BITS: {
            return 0;
        }

        case CALL_BITS: {
            return 0;
        }
    }

    return -1;
} 

int not_opcode(vm_t* vm, instruction_t instruction) {
    reg_t* dst = register_of_int32(vm, instruction, 25);
    bool_t is_register = (instruction >> 24) & 1;
    if (is_register) {
        reg_t* src = register_of_int32(vm, instruction, 19);
        *dst = -(*src);
    } else {

    }
    return 0;
}

uint32_t signed_extend21(uint32_t litteral, bool_t is_signed_extend) {
    const uint32_t bits_21_mask = 0x00100000;
    const uint32_t eleven_first_mask = 0xFFE00000;
    if (is_set(litteral, bits_21_mask)) {
        return eleven_first_mask | litteral;
    } else {
        return litteral;
    }
}

void mov_instruction(vm_t* vm, instruction_t instruction) {
    const uint32_t is_litteral_mask = 0x04000000;
    const uint32_t is_signed_extended_mask = 0x02000000;
    const uint32_t register_destination_mask = 0x01E00000;
    const uint32_t register_origin_mask = 0x001E0000;
    const uint32_t litteral_src_origin = 0x001FFFFF;
    reg_t* dst = register_of_int32(
            vm, instruction & register_destination_mask, 
            21);

    if (instruction & is_litteral_mask) {
        uint32_t litteral = instruction & litteral_src_origin;
        litteral = signed_extend21(litteral, instruction & is_signed_extended_mask);
        *dst = litteral;
    } else {

        reg_t* src = register_of_int32(
            vm, instruction & register_origin_mask, 
            17);
            *dst = *src;
    }
    printf("register value = %lld\n", ((int64_t) *dst) );
}


int vm_run(vm_t* vm){

    while (true) {
        instruction_t instruction = fetch_instruction(vm);
        opcode_t ist = opcode_value(instruction);
        switch (ist) { 
            case HALT:
                return halt_opcode(vm, instruction); 
            default:
                fprintf(stderr, "Unknown opcode %u\n", ist);
                failwith("", 1);
            break;
        }
    }
    return 0;
}


void free_vm(vm_t* vm){
    free_stack(vm->stack);
    free(vm);
}