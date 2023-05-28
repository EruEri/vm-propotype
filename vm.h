#ifndef VM_H
#define VM_H


#include "stack.h"
#include <stdint.h>

// 
// Mov
// 0000_1 
// Opcode 5bits
// Src litteral 1 bit // 1 vrai - 0 faux
// signed extend 1 : 1 vrai - 0 faux
// Reg dest = 4 bits
// reg 4 bits | src 21 bits sign extend

// Movn 
// 0001_0 
// OpCode 5bits
// Src litteral 1 bits // 1 - 0
// Rotation = 6 bits
// Reg dest = 4 bits
// reg 4 bits |  src 16 bits

// ARI
// 0001_1
// OpCode 5bits
// Src litteral 1 bits  1 vrai - 0 faux
// is_signed 1 : 1 vrai - 0 faux
// Op_arithmetic: 3 bits
// Destination: 4 bits
// Reg 4 bits | src 18 bits

// + - * / >> << % 

typedef enum {
    HALT = 0,
    MOV = 1,
    MOVN = 2,
    ARI = 3,
} instruction_set_t;


typedef enum {
    ADD = 1,
    SUB,
    MULt,
    DIV,
    MODULO,
    SHIFTLEFT,
    SHIFTRIGHT
} arithmetic_set_t;

typedef enum {
    ALWAYS,
    EQUAL,
    DIFF,
    SUP,
    UNSIGNED_SUP,
    INF,
    UNSIGNED_INF
} condition_code_t;

typedef uint64_t reg_t;
typedef uint64_t freg_t;
typedef uint32_t instruction_t;

typedef struct vm_return_t {
    int status; // 0 == success, -1 erreur
    struct {
        uint32_t op;
        const char*const message;
    } reason;
} vm_return_t;

typedef struct {
    instruction_t const * const code;
    const instruction_t* ip;
    vm_stack_t* stack;
    reg_t fp;

    // Register parameters
    reg_t r0;
    reg_t r1;
    reg_t r2;
    reg_t r3;
    reg_t r4;
    reg_t r5;
    reg_t r6;
    reg_t r7;

    // Indirect return register
    reg_t ir;
    // Syscall code register
    reg_t sc;

    reg_t r8;
    reg_t r9;
    reg_t r10;
    reg_t r11;
    reg_t r12;

    // Register parameters float
    freg_t fr0;
    freg_t fr1;
    freg_t fr2;
    freg_t fr3;
    freg_t fr4;
    freg_t fr5;
    freg_t fr6;
    freg_t fr7;

    freg_t fr8;
    freg_t fr9;
    freg_t fr10;
    freg_t fr11;
    freg_t fr12;
} vm_t;


vm_t* vm_init(instruction_t const * const code, uint64_t stack_size, uint64_t offset); 
int vm_run(vm_t* vm);
void free_vm(vm_t* vm);
#endif