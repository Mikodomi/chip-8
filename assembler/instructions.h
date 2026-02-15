#ifndef CHIP8_ASSEMBLER_INSTRUCTIONS_H
#define CHIP8_ASSEMBLER_INSTRUCTIONS_H
#include <stdio.h>
#include "tokenizer.h"

enum instr_flags_enum {
    INSTR_FLAG_CLEAR = 0x00E0,
    INSTR_FLAG_RET = 0x00EE,
    INSTR_FLAG_JMP = 0x1000,
    INSTR_FLAG_CALL = 0x2000,
    INSTR_FLAG_MOV_I_NNN = 0xA000,
    INSTR_FLAG_MOV_VX_NN = 0x6000,
    INSTR_FLAG_MOV_VX_VY = 0x8000,
    INSTR_FLAG_MOV_VX_DTM = 0xF007,
    INSTR_FLAG_MOV_DTM_VX = 0xF015,
    INSTR_FLAG_MOV_STM_VX = 0xF018,
    INSTR_FLAG_ADD_VX_VY = 0x8004,
    INSTR_FLAG_ADD_VX_NN = 0x7000,
    INSTR_FLAG_ADD_I_VX = 0xF01E,
    INSTR_FLAG_JV0 = 0xB000,
    INSTR_FLAG_OR = 0x8001,
    INSTR_FLAG_AND = 0x8002,
    INSTR_FLAG_XOR = 0x8003,
    INSTR_FLAG_SUB = 0x8005,
    INSTR_FLAG_MSR = 0x8006,
    INSTR_FLAG_SUBS = 0x8007,
    INSTR_FLAG_MSL = 0x800E,
    INSTR_FLAG_SKPE_VX_NN = 0x3000,
    INSTR_FLAG_SKPE_VX_VY = 0x5000,
    INSTR_FLAG_SKPE_VX = 0xE09E,
    INSTR_FLAG_SKPNE_VX_NN = 0x4000,
    INSTR_FLAG_SKPNE_VX_VY = 0x9000,
    INSTR_FLAG_SKPNE_VX = 0xE0A1,
    INSTR_FLAG_RAND = 0xC000,
    INSTR_FLAG_DRAW = 0xD000,
    INSTR_FLAG_IN = 0xF00A,
    INSTR_FLAG_HH5 = 0xF029,
    INSTR_FLAG_BCD = 0xF033,
    INSTR_FLAG_MOVOUT = 0xF055,
    INSTR_FLAG_MOVIN = 0xF065,
};

typedef enum instr_flags_enum instr_flags;


enum error_t_enum {
    SUCCESS,
    INVALID_OPERANDS,
    INVALID_OPERAND_SIZE,
    REGEX_ERROR,
};

typedef enum error_t_enum error_t;

int assemble(FILE* in, FILE* out);

int parse_number(char* word);

//error_t reg_reg_instr(FILE* out, char* op1, char* op2, instr_flags flag);
//error_t mem_instr(FILE* out, char* line, instr_flags instr);

error_t reg_reg_instr(FILE* out, const token* token_arr);
error_t mem_instr(FILE* out, const token* token_arr);

error_t mov(FILE* out, const token* token_arr);
error_t skpe(FILE* out, const token* token_arr);
error_t skpne(FILE* out, const token* token_arr);
error_t add(FILE* out, const token* token_arr);
error_t rand_chip8(FILE* out, const token* token_arr); // this makes me angry
error_t draw(FILE* out, const token* token_arr);


//error_t mov(FILE* out, char* op1, char* op2);

#endif
