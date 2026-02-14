#ifndef CHIP8_ASSEMBLER_INSTRUCTIONS_H
#define CHIP8_ASSEMBLER_INSTRUCTIONS_H
#include <stdio.h>

enum instr_flags_enum {
    INSTR_FLAG_JMP = 0x1000,
    INSTR_FLAG_CALL = 0x2000,
};

typedef enum instr_flags_enum instr_flags;

int assemble(FILE* in, FILE* out);

int parse_number(char* word);

int jmp_call(FILE* out, char* line, instr_flags instr);

#endif
