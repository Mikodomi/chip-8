#include <SDL3/SDL.h>
#include <string.h>
#include <inttypes.h>
#include <stdio.h>
#include <errno.h>

#define MEM_SIZE 4096
#define REG_COUNT 16

// takes original 16 bits as input (in big endian order
#define REG1(x) (x & 0x000F)
#define REG2(x) ((x & 0xF000) >> 8)
#define CONST_VALUE(x) ((x & 0xFF00) >> 8)
#define MEM_VALUE(x) (((x & 0xFF00) >> 8) | ((x & 0x000F) << 8))

typedef struct {
    uint8_t     mem[MEM_SIZE];
    uint8_t     *stack_pointer;
    uint8_t     v[REG_COUNT]; // data registers
    uint16_t    address; // 12-bit address register
    uint16_t    pc; // program counter

    uint16_t size;
} chip8;

int chip8_load_rom(chip8* machine, FILE* rom_input) {
    memset(machine->mem, 0, MEM_SIZE);
    size_t bytes_read = fread(machine->mem, sizeof(char), MEM_SIZE, rom_input);
    machine->size = bytes_read;
    return bytes_read;
}

int chip8_decode_2reg(chip8* machine, uint16_t instruction) {
    size_t reg1 = REG1(instruction), reg2 = REG2(instruction);
    uint8_t val1 = machine->v[reg1], val2 = machine->v[reg2];
    uint8_t operation = (instruction & 0x0F00) >> 8;
    switch (operation) {
        case 0:
            val1 = val2;
            break;
        case 1:
            val1 |= val2;
            break;
        case 2:
            val1 &= val2;
            break;
        case 3:
            val1 ^= val2;
            break;
        case 4:
            val1 += val2;
            machine->v[0x0F] = (val1 + val2) > 0xFF;
            break;
        case 5:
            machine->v[0x0F] = (val1 >= val2);
            val1 -= val2;
            break;
        case 6:
            machine->v[0x0F] = (val1 & 0x01);
            val1 >>= 1;
            break;
        case 7:
            machine->v[0x0F] = (val2 >= val1);
            val1 = val2 - val1;
            break;
        case 0x0E:
            machine->v[0x0F] = (val1 & 0x80);
            val1 <<= 1;
            break;
        default:
            return -1;
    }
    machine->v[reg1] = val1;
    return 0;
}

int chip8_decode_zeroes(chip8* machine, uint16_t instruction) {
    int status = 0;
    uint8_t bottom_bits = MEM_VALUE(instruction);
    switch (bottom_bits) {
        case 0x0E0:
            break;
        case 0x0EE:
            machine->pc = (*machine->stack_pointer) / 2;
            *machine->stack_pointer -= 2;
            break;
        default:
            machine->pc = bottom_bits / 2;
            break; // what does "jump to native assembler subroutine" even mean?
    }
    return status;
}

int chip8_decode_execute(chip8* machine, uint16_t instruction) {
    int status = 0;
    size_t reg1, reg2;
    uint8_t value;
    uint8_t first_4_bits = (instruction & 0x00F0) >> 4;
    uint8_t zero;
    uint16_t addr;
    switch (first_4_bits) {
        case 0: 
            status = chip8_decode_zeroes(machine, instruction);
            break;
        case 1: 
            addr = MEM_VALUE(instruction);
            machine->pc = addr / 2;
            break;
        case 2: 
            *machine->stack_pointer += 2;
            *machine->stack_pointer = machine->pc * 2;
            addr = MEM_VALUE(instruction);
            machine->pc = addr / 2;
            break;
        case 3: 
            reg1 = REG1(instruction);
            value = CONST_VALUE(instruction);
            machine->pc += (machine->v[reg1] == value);
            break;
        case 4: 
            reg1 = REG1(instruction);
            value = CONST_VALUE(instruction);
            machine->pc += (machine->v[reg1] != value);
            break;
        case 5: 
            zero = (instruction & 0x0F00) >> 8;
            if (zero != 0) { status = -1; break; }
            reg1 = REG1(instruction);
            reg2 = REG2(instruction);
            machine->pc += (machine->v[reg1] == machine->v[reg2]);
            break;
        case 6: 
            reg1 = REG1(instruction);
            value = CONST_VALUE(instruction); 
            machine->v[reg1] = value;
            break;
        case 7: 
            reg1 = REG1(instruction);
            value = CONST_VALUE(instruction);
            machine->v[reg1] += value;
            break;
        case 8: 
            status = chip8_decode_2reg(machine, instruction);
            break;
        case 9: 
            zero = (instruction & 0x0F00) >> 8;
            if (zero != 0) { status = -1; break; }
            reg1 = REG1(instruction);
            reg2 = REG2(instruction);
            machine->pc += (machine->v[reg1] != machine->v[reg2]);
            break;
        case 0xA: 
            addr = MEM_VALUE(instruction);      
            machine->address = addr;
            break;
        case 0xB: break;
        case 0xC: break;
        case 0xD: break;
        case 0xE: break;
        case 0xF: break;

        default:
            status = -1;
    }
    return status;
}


int chip8_fde_cycle(chip8* machine) { // fetch decode execute
    machine->pc = 0;
    machine->stack_pointer = &machine->mem[0xEA0];
    int status = 0;
    uint16_t instruction;
    for (;2*machine->pc < machine->size;) {
        instruction = ((uint16_t*)machine->mem)[machine->pc]; 
        machine->pc++;
        status = chip8_decode_execute(machine, instruction);
        if (status != 0) break;
    }
    return status;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("usage: %s <rom_name>\n", argv[0]);
        return -1;
    }
    FILE* rom = fopen(argv[1], "rb");
    if (rom == NULL) {
        perror("fopen");
        return -1;
    }

    chip8 machine;
    chip8_load_rom(&machine, rom);
    chip8_fde_cycle(&machine);
    
    fclose(rom);  
    return 0;
}
