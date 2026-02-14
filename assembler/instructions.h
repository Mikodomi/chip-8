#ifndef CHIP8_ASSEMBLER_INSTRUCTIONS_H
#define CHIP8_ASSEMBLER_INSTRUCTIONS_H
#include <stdio.h>

enum instr_flags_enum {
    INSTR_FLAG_JMP = 0x1000,
    INSTR_FLAG_CALL = 0x2000,
    INSTR_FLAG_MOV_I = 0xA000,
    INSTR_FLAG_JV0 = 0xB000,
    INSTR_FLAG_REG_REG_MOV = 0x8000,
    INSTR_FLAG_REG_REG_OR = 0x8001,
    INSTR_FLAG_REG_REG_AND = 0x8002,
    INSTR_FLAG_REG_REG_XOR = 0x8003,
    INSTR_FLAG_REG_REG_ADD = 0x8004,
    INSTR_FLAG_REG_REG_SUB = 0x8005,
    INSTR_FLAG_REG_REG_MSR = 0x8006,
    INSTR_FLAG_REG_REG_SUBS = 0x8007,
    INSTR_FLAG_REG_REG_MSL = 0x800E,
};

typedef enum instr_flags_enum instr_flags;

int assemble(FILE* in, FILE* out);

int parse_number(char* word);

int reg_reg_instr(FILE* out, char* op1, char* op2, instr_flags flag);
int mem_instr(FILE* out, char* line, instr_flags instr);

int mov(FILE* out, char* op1, char* op2);

#endif
