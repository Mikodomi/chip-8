#include "instructions.h"
#include "tokenizer.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <inttypes.h>
#include <regex.h>

size_t write_BE(FILE* out, uint16_t in) {
    char temp[2];
    temp[0] = (in & 0xFF00) >> 8;
    temp[1] = (in & 0x00FF);
    return fwrite(temp, 1, 2, out);
}

int assemble(FILE* in, FILE* out) {
    size_t len = 0;
    ssize_t nread;
    char* lineptr;
    int amount;
    while ((nread = getline(&lineptr, &len, in)) != -1) {
        if (lineptr[0] == ';' || lineptr[0] == '\n') continue;
        token token_arr[MAX_TOKENS];
        amount = tokenize_line(lineptr, token_arr);  
        // todo: error handling
        switch(token_arr[0].type) {
                case TOKEN_CLEAR:
                    write_BE(out, INSTR_FLAG_CLEAR);
                    break;
                case TOKEN_RET:
                    write_BE(out, INSTR_FLAG_RET);
                    break;
                case TOKEN_JMP:
                case TOKEN_CALL:
                case TOKEN_JV0:
                    mem_instr(out, token_arr);
                    break;
                case TOKEN_MOV:
                    mov(out, token_arr);
                    break;
                case TOKEN_OR:
                case TOKEN_AND:
                case TOKEN_XOR:
                case TOKEN_SUB:
                case TOKEN_SUBS:
                case TOKEN_MSR:
                case TOKEN_MSL:
                    reg_reg_instr(out, token_arr);
                    break;
                case TOKEN_SKPE:
                    skpe(out, token_arr);
                    break;
                case TOKEN_SKPNE:
                    skpne(out, token_arr);
                    break;
                case TOKEN_ADD:
                    add(out, token_arr);
                    break;
                case TOKEN_RAND:
                    rand_chip8(out, token_arr);
                    break;
                case TOKEN_DRAW:
                    draw(out, token_arr);
                    break;
                case TOKEN_IN: 
                case TOKEN_HH5:
                case TOKEN_BCD:
                case TOKEN_MOVOUT:
                case TOKEN_MOVIN:
                    unique_F(out, token_arr);
                    break;
                default: return -1;
        }
    }
    return 0;
}

error_t mem_instr(FILE* out, const token* token_arr) {
    if (token_arr[1].type != TOKEN_CONST) return INVALID_OPERANDS;
    if (token_arr[1].value >= 0x1000) return INVALID_OPERAND_SIZE;
    uint16_t flag;
    switch (token_arr[0].type) {
        case TOKEN_JMP: flag = INSTR_FLAG_JMP; break;
        case TOKEN_CALL: flag = INSTR_FLAG_CALL; break;
        case TOKEN_JV0: flag = INSTR_FLAG_JV0; break;
        default: return INVALID_OPERANDS;
    }
    uint16_t opcode = token_arr[1].value;
    opcode |= INSTR_FLAG_JMP;
    return write_BE(out, opcode) == 2;
}

error_t mov(FILE* out, const token* token_arr) {
    uint16_t opcode;
    switch (token_arr[1].type) {
        case TOKEN_I:
            if (token_arr[2].type != TOKEN_CONST) return INVALID_OPERANDS;
            if (token_arr[2].value >= 0x1000) return INVALID_OPERAND_SIZE;
            opcode = token_arr[2].value;
            opcode |= INSTR_FLAG_MOV_I_NNN;
            break;
        case TOKEN_REG:
            opcode = (token_arr[1].value) << 8;
            switch(token_arr[2].type) {
                case TOKEN_REG:
                    opcode |= ((token_arr[2].value) << 4);
                    opcode |= INSTR_FLAG_MOV_VX_VY;
                    break;
                case TOKEN_CONST:
                    if (token_arr[2].value >= 0x100) return INVALID_OPERAND_SIZE;
                    opcode |= INSTR_FLAG_MOV_VX_NN;
                    opcode |= (token_arr[2].value); 
                    break;
                case TOKEN_DTM:
                    opcode |= INSTR_FLAG_MOV_VX_DTM;
                    break;
                default: return INVALID_OPERANDS;
            }
            break;
        case TOKEN_DTM:
            if (token_arr[2].type != TOKEN_REG) return INVALID_OPERANDS;
            opcode = (token_arr[2].value) << 8;
            opcode |= INSTR_FLAG_MOV_DTM_VX;
            break;
        case TOKEN_STM:
            if (token_arr[2].type != TOKEN_REG) return INVALID_OPERANDS;
            opcode = (token_arr[2].value) << 8;
            opcode |= INSTR_FLAG_MOV_STM_VX;
            break;
        default:
            return INVALID_OPERANDS;
    } 
    return write_BE(out, opcode) == 2;
}

error_t reg_reg_instr(FILE* out, const token* token_arr) {
    if (token_arr[1].type != TOKEN_REG || token_arr[2].type != TOKEN_REG) {
        return INVALID_OPERANDS;
    }
    uint16_t opcode;
    opcode = (token_arr[1].value << 8) | (token_arr[2].value << 4);
    switch (token_arr[0].type) {
        // mov implemented seperately
        case TOKEN_OR: opcode |= INSTR_FLAG_OR; break;
        case TOKEN_AND: opcode |= INSTR_FLAG_AND; break;
        case TOKEN_XOR: opcode |= INSTR_FLAG_XOR; break;
        // add implemented seperately
        case TOKEN_SUB: opcode |= INSTR_FLAG_SUB; break;
        case TOKEN_MSR: opcode |= INSTR_FLAG_MSR; break;
        case TOKEN_SUBS: opcode |= INSTR_FLAG_SUBS; break;
        case TOKEN_MSL: opcode |= INSTR_FLAG_MSL; break;
        // skpe and skpne implemented seperately
        default: return INVALID_OPERANDS;
    }
    return write_BE(out, opcode) == 2;
}

error_t skpe(FILE* out, const token* token_arr) {
    if (token_arr[0].type != TOKEN_SKPE) return INVALID_OPERANDS;
    if (token_arr[1].type != TOKEN_REG) return INVALID_OPERANDS;
    uint16_t opcode = (token_arr[1].value << 8);
    switch (token_arr[2].type) {
        case TOKEN_CONST:
            if (token_arr[2].value >= 0x100) return INVALID_OPERAND_SIZE;
            opcode |= token_arr[2].value;
            opcode |= INSTR_FLAG_SKPE_VX_NN;
            break;
        case TOKEN_REG:
            opcode |= (token_arr[2].value << 4);
            opcode |= INSTR_FLAG_SKPE_VX_VY;
            break;
        case TOKEN_EMPTY:
            opcode |= INSTR_FLAG_SKPE_VX;
            break;
        default: return INVALID_OPERANDS;
    }
    return write_BE(out, opcode) == 2;
}

error_t skpne(FILE* out, const token* token_arr) {
    if (token_arr[0].type != TOKEN_SKPNE) return INVALID_OPERANDS;
    if (token_arr[1].type != TOKEN_REG) return INVALID_OPERANDS;
    uint16_t opcode = (token_arr[1].value << 8);
    switch (token_arr[2].type) {
        case TOKEN_CONST:
            if (token_arr[2].value >= 0x100) return INVALID_OPERAND_SIZE;
            opcode |= token_arr[2].value;
            opcode |= INSTR_FLAG_SKPNE_VX_NN;
            break;
        case TOKEN_REG:
            opcode |= (token_arr[2].value << 4);
            opcode |= INSTR_FLAG_SKPNE_VX_VY;
            break;
        case TOKEN_EMPTY:
            opcode |= INSTR_FLAG_SKPNE_VX;
            break;
        default: return INVALID_OPERANDS;
    }
    return write_BE(out, opcode) == 2;
}

error_t add(FILE* out, const token* token_arr) {
    if (token_arr[0].type != TOKEN_ADD) return INVALID_OPERANDS;
    uint16_t opcode;
    switch (token_arr[1].type) {
        case TOKEN_REG:
            opcode = (token_arr[1].value << 8);
            switch (token_arr[2].type) {
                case TOKEN_REG:
                    opcode |= (token_arr[2].value << 4);
                    opcode |= INSTR_FLAG_ADD_VX_VY;
                    break;
                case TOKEN_CONST:
                    if (token_arr[2].value >= 0x100) return INVALID_OPERAND_SIZE;
                    opcode |= (token_arr[2].value);
                    opcode |= INSTR_FLAG_ADD_VX_NN;
                    break;
                default:
                    return INVALID_OPERANDS;
                }
            break;
        case TOKEN_I:
            if (token_arr[2].type != TOKEN_REG) return INVALID_OPERANDS;
            opcode = INSTR_FLAG_ADD_I_VX;
            opcode |= (token_arr[2].value << 8);
            break;
        default: return INVALID_OPERANDS;
    }
    return write_BE(out, opcode) == 2;
}

error_t rand_chip8(FILE* out, const token* token_arr) {
    if (token_arr[0].type != TOKEN_RAND) return INVALID_OPERANDS;     
    if (token_arr[1].type != TOKEN_REG) return INVALID_OPERANDS;     
    if (token_arr[2].type != TOKEN_CONST) return INVALID_OPERANDS;     
    if (token_arr[2].value >= 0x100) return INVALID_OPERAND_SIZE;
    uint16_t opcode = INSTR_FLAG_RAND;
    opcode |= (token_arr[1].value << 8);
    opcode |= (token_arr[2].value);
    return write_BE(out, opcode) == 2;
}

error_t draw(FILE* out, const token* token_arr) {
    if (token_arr[0].type != TOKEN_DRAW) return INVALID_OPERANDS;
    if (token_arr[1].type != TOKEN_REG && token_arr[2].type != TOKEN_REG) return INVALID_OPERANDS;
    if (token_arr[3].type != TOKEN_CONST) return INVALID_OPERANDS;
    if (token_arr[3].value >= 0x10) return INVALID_OPERAND_SIZE;
    uint16_t opcode = INSTR_FLAG_DRAW;
    opcode |= (token_arr[1].value << 8);
    opcode |= (token_arr[2].value << 4);
    opcode |= (token_arr[3].value);
    return write_BE(out, opcode) == 2;
}

error_t unique_F(FILE* out, const token* token_arr) {
    uint16_t opcode;
    switch (token_arr[0].type) {
        case TOKEN_IN: opcode = INSTR_FLAG_IN; break;
        case TOKEN_HH5: opcode = INSTR_FLAG_HH5; break;
        case TOKEN_BCD: opcode = INSTR_FLAG_BCD; break;
        case TOKEN_MOVOUT: opcode = INSTR_FLAG_MOVOUT; break;
        case TOKEN_MOVIN: opcode = INSTR_FLAG_MOVIN; break;
        default:
            return INVALID_OPERANDS;
    }
    if (token_arr[1].type != TOKEN_REG) return INVALID_OPERANDS;
    opcode |= (token_arr[1].value << 8);
    return write_BE(out, opcode) == 2;
}
