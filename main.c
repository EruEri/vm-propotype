#include "vm.h"
#include <stddef.h>
#include <stdio.h>


const instruction_t code [] = {
    0b00001100000000000000000000001111,
    0x00
};

int main() {
    vm_t* vm = vm_init(code, 16, 0);
    int status = vm_run(vm);
    free_vm(vm);
    return status;
}