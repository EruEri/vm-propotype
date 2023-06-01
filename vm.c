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
#define CC_ONLY_MASK 0xF
#define BR_JMP_MASK 0X3

#define opcode_value(instruction) \
    (((uint32_t) instruction & OPCODE_MASK) >> (INSTRUCTION_SIZE - OPCODE_SIZE))

#define is_set(instruction, mask) \
    ((instruction & mask) == mask)

#define mask_bit(n) \
    (1 << n)

vm_t* vm_init(const instruction_t *const code, uint64_t stack_size, uint64_t offset) {
    vm_t* vm_ptr = malloc(sizeof(vm_t));
    if (!vm_ptr) failwith("Vm alloc fail", 1);
    vm_stack_t* stack = stack_create(stack_size);
    const instruction_t* ip = code + offset;
    vm_t vm = {.stack = stack, .code = code, .ip = ip, .fp = stack->sp, .last_cmp = false};
    memcpy(vm_ptr, &vm, sizeof(vm_t));
    return vm_ptr;
}

instruction_t fetch_instruction(vm_t* vm) {
    return *(vm->ip++);
}

int64_t signed_extend25(instruction_t instruction, bool_t is_sign_extended) {
    const uint32_t seven_first_mask = 0xFE000000;
    const uint32_t litteral = instruction & (~seven_first_mask);
    if (is_set(instruction, mask_bit(25))) {
        return seven_first_mask | litteral;
    } else {
        return litteral;
    }
}

int64_t signed_extend22(instruction_t instruction, bool_t is_signed_extend) {
    const uint32_t bits_22_mask = mask_bit(22);
    const uint32_t ten_first_mask = 0xFFC00000;
    const uint32_t litteral = instruction & (~ten_first_mask);

    if (is_set(litteral, bits_22_mask)) {
        return ten_first_mask | litteral;
    } else {
        return litteral;
    }
}

int64_t signed_extend21(instruction_t instruction, bool_t is_signed_extend) {
    const uint32_t bits_21_mask = mask_bit(21);
    const uint32_t eleven_first_mask = 0xFFE00000;
    const uint32_t litteral = instruction & (~eleven_first_mask);

    if (is_set(litteral, bits_21_mask)) {
        return eleven_first_mask | litteral;
    } else {
        return litteral;
    }
}

int64_t signed_extend18(instruction_t instruction, bool_t is_signed_extend) {
    const uint32_t fourteen_first_mask = 0xFFFA0000;
    const uint32_t litteral = instruction & ~fourteen_first_mask;

    if (is_set(instruction, mask_bit(18))) {
        return fourteen_first_mask | litteral;
    } else {
        return litteral;
    }
}

int64_t signed_extend16(instruction_t instruction, bool_t is_signed_extend) {
    const uint32_t sixteen_first_mask = 0xFFFF0000;
    const uint32_t litteral = instruction & ~sixteen_first_mask;

    if (is_set(instruction, mask_bit(16))) {
        return sixteen_first_mask | litteral;
    } else {
        return litteral;
    }
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

int mvnt(vm_t* vm, instruction_t instruction) {
    reg_t* dst = register_of_int32(vm, instruction, 27);
    bool_t is_register = (instruction >> 26) & 1;
    if (is_register) {
        reg_t* src = register_of_int32(vm, instruction, 21);
        *dst = ~(*src);
    } else {
        bool_t is_signed = is_set(instruction, mask_bit(25));
        int64_t value = signed_extend21(instruction, is_signed);
        *dst = ~value;
    }
    return 0;
}

int mvng(vm_t* vm, instruction_t instruction) {
    reg_t* dst = register_of_int32(vm, instruction, 22);
    bool_t is_register = (instruction >> 21) & 1;
    if (is_register) {
        reg_t* src = register_of_int32(vm, instruction, 16);
        *dst = -(*src);
    } else {
        bool_t is_signed = is_set(instruction, mask_bit(20));
        int64_t value = signed_extend21(instruction, is_signed);
        *dst = -value;
    }
    return 0;
}

int mv(vm_t* vm, instruction_t instruction) {
    reg_t* dst = register_of_int32(vm, instruction, 22);
    bool_t is_register = (instruction >> 21) & 1;
    if (is_register) {
        reg_t* src = register_of_int32(vm, instruction, 16);
        *dst = (*src);
    } else {
        bool_t is_signed = is_set(instruction, mask_bit(20));
        int64_t value = signed_extend21(instruction, is_signed);
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
        int64_t value = signed_extend18(instruction, is_signed);
        *dst = *dst | (value << shift);
    } 
    return 0;
}

int br(vm_t* vm, instruction_t instruction) {
    bool_t is_branch_link = is_set(instruction, mask_bit(26));
    bool_t is_register = is_set(instruction, mask_bit(25));
    if (is_register) {
       reg_t* src = register_of_int32(vm, instruction, 20);
       if (is_branch_link) {
            vm->fp = (reg_t) vm->ip;
       }
       vm->ip = vm->code + *src;
    } else {
        bool_t is_signed = is_set(instruction, mask_bit(24));
        int64_t value = signed_extend25(instruction, is_signed);
        if (is_branch_link) {
            vm->fp = (reg_t) vm->ip;
        }
        vm->ip = vm->ip + value;
    }

    return 0;
}

int lea(vm_t* vm, instruction_t instruction) {
    reg_t* dst = register_of_int32(vm, instruction, 22);
    bool_t is_signed = is_set(instruction, mask_bit(21));
    int64_t value = signed_extend22(instruction, is_signed);
    *dst = (reg_t) vm->ip + value;
    return 0;
}

int add(vm_t* vm, instruction_t instruction) {
    reg_t* dst = register_of_int32(vm, instruction, 22);
    reg_t* src = register_of_int32(vm, instruction, 17);
    bool_t is_register = is_set(instruction, mask_bit(16));
    if (is_register) {
        reg_t* src2 = register_of_int32(vm, instruction, 11);
        *dst = *src + *src2;
    } else {
        bool_t is_signed = is_set(instruction, mask_bit(15));
        int64_t value = signed_extend16(instruction, is_signed);
        *dst = *src + value;
    }
    return 0;
}


int sub(vm_t* vm, instruction_t instruction) {
    reg_t* dst = register_of_int32(vm, instruction, 22);
    reg_t* src = register_of_int32(vm, instruction, 17);
    bool_t is_register = is_set(instruction, mask_bit(16));
    if (is_register) {
        reg_t* src2 = register_of_int32(vm, instruction, 11);
        *dst = *src - *src2;
    } else {
        bool_t is_signed = is_set(instruction, mask_bit(15));
        int64_t value = signed_extend16(instruction, is_signed);
        *dst = *src - value;
    }
    return 0;
}

int mult(vm_t* vm, instruction_t instruction) {
    reg_t* dst = register_of_int32(vm, instruction, 22);
    reg_t* src = register_of_int32(vm, instruction, 17);
    bool_t is_register = is_set(instruction, mask_bit(16));
    if (is_register) {
        reg_t* src2 = register_of_int32(vm, instruction, 11);
        *dst = *src * *src2;
    } else {
        bool_t is_signed = is_set(instruction, mask_bit(15));
        int64_t value = signed_extend16(instruction, is_signed);
        *dst = *src * value;
    }
    return 0;
}

int idiv(vm_t* vm, instruction_t instruction) {
    return -1;
}

int mod(vm_t* vm, instruction_t instruction) {
    return -1;
}


int iand(vm_t* vm, instruction_t instruction) {
    reg_t* dst = register_of_int32(vm, instruction, 22);
    reg_t* src = register_of_int32(vm, instruction, 17);
    bool_t is_register = is_set(instruction, mask_bit(16));
    if (is_register) {
        reg_t* src2 = register_of_int32(vm, instruction, 11);
        *dst = *src & *src2;
    } else {
        bool_t is_signed = is_set(instruction, mask_bit(15));
        int64_t value = signed_extend16(instruction, is_signed);
        *dst = *src & value;
    }
    return 0;
}

int ior(vm_t* vm, instruction_t instruction) {
    reg_t* dst = register_of_int32(vm, instruction, 22);
    reg_t* src = register_of_int32(vm, instruction, 17);
    bool_t is_register = is_set(instruction, mask_bit(16));
    if (is_register) {
        reg_t* src2 = register_of_int32(vm, instruction, 11);
        *dst = *src | *src2;
    } else {
        bool_t is_signed = is_set(instruction, mask_bit(15));
        int64_t value = signed_extend16(instruction, is_signed);
        *dst = *src | value;
    }
    return 0;
}

int ixor(vm_t* vm, instruction_t instruction) {
    reg_t* dst = register_of_int32(vm, instruction, 22);
    reg_t* src = register_of_int32(vm, instruction, 17);
    bool_t is_register = is_set(instruction, mask_bit(16));
    if (is_register) {
        reg_t* src2 = register_of_int32(vm, instruction, 11);
        *dst = *src ^ *src2;
    } else {
        bool_t is_signed = is_set(instruction, mask_bit(15));
        int64_t value = signed_extend16(instruction, is_signed);
        *dst = *src ^ value;
    }
    return 0;
}

int ilsl(vm_t* vm, instruction_t instruction) {
    reg_t* dst = register_of_int32(vm, instruction, 22);
    reg_t* src = register_of_int32(vm, instruction, 17);
    bool_t is_register = is_set(instruction, mask_bit(16));
    if (is_register) {
        reg_t* src2 = register_of_int32(vm, instruction, 11);
        *dst = *src << *src2;
    } else {
        bool_t is_signed = is_set(instruction, mask_bit(15));
        int64_t value = signed_extend16(instruction, is_signed);
        *dst = *src << value;
    }
    return 0;
}

int iasr(vm_t* vm, instruction_t instruction) {
    reg_t* dst = register_of_int32(vm, instruction, 22);
    reg_t* src = register_of_int32(vm, instruction, 17);
    bool_t is_register = is_set(instruction, mask_bit(16));
    if (is_register) {
        reg_t* src2 = register_of_int32(vm, instruction, 11);
        *dst = ((int64_t) *src) >> *src2;
    } else {
        bool_t is_signed = is_set(instruction, mask_bit(15));
        int64_t value = signed_extend16(instruction, is_signed);
        *dst = *src << value;
    }
    return 0;
}

int ilsr(vm_t* vm, instruction_t instruction) {
    reg_t* dst = register_of_int32(vm, instruction, 22);
    reg_t* src = register_of_int32(vm, instruction, 17);
    bool_t is_register = is_set(instruction, mask_bit(16));
    if (is_register) {
        reg_t* src2 = register_of_int32(vm, instruction, 11);
        *dst = ((uint64_t) *src) >> *src2;
    } else {
        bool_t is_signed = is_set(instruction, mask_bit(15));
        int64_t value = signed_extend16(instruction, is_signed);
        *dst = *src << value;
    }
    return 0;
}

bool_t cmp_value(condition_code_t cc, reg_t lhs, reg_t rhs) {
    switch (cc) {
    case ALWAYS:
        return true;
    case EQUAL:
        return lhs == rhs;
    case DIFF:
        return lhs != rhs;
    case SUP:
        return (int64_t) lhs > (int64_t) rhs;
    case UNSIGNED_SUP:
        return lhs > rhs;
    case SUPEQ:
        return (int64_t) lhs >= (int64_t) rhs;
    case UNSIGNED_SUPEQ:
        return lhs >= rhs;
    case INF:
        return (int64_t) lhs < (int64_t) rhs;
    case UNSIGNED_INF:
        return lhs < rhs;
    case INFEQ:
        return (int64_t) lhs <= (int64_t) rhs;
    case UNSIGNED_INFEQ:
        return lhs <= rhs;
    default:
        return false;
    }
}

int cmp(vm_t* vm, instruction_t instruction) {
    condition_code_t cc = (instruction >> 5) & CC_ONLY_MASK;
    bool_t is_cset = is_set(instruction, mask_bit(23));
    reg_t* reg1 = register_of_int32(vm, instruction, 22);
    reg_t* reg2 = register_of_int32(vm, instruction, 17);
    bool_t value = cmp_value(cc, *reg1, *reg2);
    if (is_cset) {
        reg_t* reg3 = register_of_int32(vm, instruction, 12);
        *reg3 = value;
    } else {
        vm->last_cmp = value;
    }

    return 0;
}

int vm_run(vm_t* vm){
    while (true) {
        instruction_t instruction = fetch_instruction(vm);
        opcode_t ist = opcode_value(instruction);
        switch (ist) { 
            case HALT:
                return halt_opcode(vm, instruction); 
            case MVNOT:
                mvnt(vm, instruction);
                break;
            case MVNEG:
                mvng(vm, instruction);
                break;
            case MOV:
                mv(vm, instruction);
                break;
            case BR_JUMP:
                br(vm, instruction);
                break;
            case LEA:
                lea(vm, instruction);
                break;
            case ADD:
                add(vm, instruction);
                break;
            case SUB:
                sub(vm, instruction);
                break;
            case MULT:
                mult(vm, instruction);
                break;
            case DIV:
                idiv(vm, instruction);
                break;
            case MOD:
                mod(vm, instruction);
                break;
            case AND:
                iand(vm, instruction);
                break;
            case OR:    
                ior(vm, instruction);
                break;
            case XOR:
                ixor(vm, instruction);
                break;
            case LSL:
                ilsl(vm, instruction);
                break;
            case ASR:
                iasr(vm, instruction);
                break;
            case LSR:
                ilsr(vm, instruction);
                break;
            case CMP:
            case CSET:
                cmp(vm, instruction);
                break;
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