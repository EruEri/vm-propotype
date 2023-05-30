#include "vm.h"
#include "stack.h"
#include "util.h"
#include <_types/_uint32_t.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HALT_BITS 0x0
#define RET_BITS 0x01
#define SYSCALL_BITS 0x2
#define CALL_BITS 0x3


#define SHIFT_ONLY_MASK 0x3
#define REG_ONLY_MASK 0x1F

#define opcode_value(instruction) \
    (((uint32_t) instruction & OPCODE_MASK) >> (INSTRUCTION_SIZE - OPCODE_SIZE))

#define is_set(instruction, mask) \
    ((instruction & mask) == mask)

#define mask_bit(n) \
    (1 << n)

uint64_t signed_extend21(instruction_t instruction, bool_t is_signed_extend);
uint64_t signed_extend18(instruction_t instruction, bool_t is_signed_extend);

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
    switch ((bits >> shift) & REG_ONLY_MASK) {
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
    reg_t* dst = register_of_int32(vm, instruction, 22);
    bool_t is_register = (instruction >> 21) & 1;
    if (is_register) {
        reg_t* src = register_of_int32(vm, instruction, 16);
        *dst = ~(*src);
    } else {
        bool_t is_signed = is_set(instruction, mask_bit(20));
        uint64_t value = signed_extend21(instruction, is_signed);
        *dst = ~value;
    }
    return 0;
}

int neg_opcode(vm_t* vm, instruction_t instruction) {
    reg_t* dst = register_of_int32(vm, instruction, 22);
    bool_t is_register = (instruction >> 21) & 1;
    if (is_register) {
        reg_t* src = register_of_int32(vm, instruction, 16);
        *dst = -(*src);
    } else {
        bool_t is_signed = is_set(instruction, mask_bit(20));
        uint64_t value = signed_extend21(instruction, is_signed);
        *dst = -value;
    }
    return 0;
}

int mv_opcode(vm_t* vm, instruction_t instruction) {
    reg_t* dst = register_of_int32(vm, instruction, 22);
    bool_t is_register = (instruction >> 21) & 1;
    if (is_register) {
        reg_t* src = register_of_int32(vm, instruction, 16);
        *dst = (*src);
    } else {
        bool_t is_signed = is_set(instruction, mask_bit(20));
        uint64_t value = signed_extend21(instruction, is_signed);
        *dst = value;
    }
    return 0;
}

int mva(vm_t* vm, instruction_t instruction) {
    reg_t* dst = register_of_int32(vm, instruction, 22);
    uint8_t shift = ((instruction >> 20) & SHIFT_ONLY_MASK) * 16;
    bool_t is_reg = is_set(instruction, mask_bit(19));
    if (is_reg) {
        reg_t* src = register_of_int32(vm, instruction, 14);
        *dst = *dst | ( *src << shift);
    } else {
        bool_t is_signed = is_set(instruction, mask_bit(18));
        uint64_t value = signed_extend18(instruction, is_signed);
        *dst = *dst | (value << shift);
    } 
    return 0;
}

uint64_t signed_extend21(instruction_t instruction, bool_t is_signed_extend) {
    const uint32_t bits_21_mask = mask_bit(21);
    const uint32_t eleven_first_mask = 0xFFE00000;
    const uint32_t litteral = instruction & (~eleven_first_mask);

    if (is_set(litteral, bits_21_mask)) {
        int64_t n = eleven_first_mask | litteral;
        return n;
    } else {
        return litteral;
    }
}

uint64_t signed_extend18(instruction_t instruction, bool_t is_signed_extend) {
    const uint32_t fourteen_first_mask = 0xFFFA0000;
    const uint32_t litteral = instruction & ~fourteen_first_mask;

    if (is_set(instruction, mask_bit(18))) {
        int64_t n = fourteen_first_mask | litteral;
        return n;
    } else {
        return litteral;
    }
}


int vm_run(vm_t* vm){

    while (true) {
        instruction_t instruction = fetch_instruction(vm);
        opcode_t ist = opcode_value(instruction);
        switch (ist) { 
            case HALT:
                return halt_opcode(vm, instruction); 
            case MVNOT:
                return not_opcode(vm, instruction);
            case MVNEG:
                return neg_opcode(vm, instruction);
            case MOV:
                return mv_opcode(vm, instruction);
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