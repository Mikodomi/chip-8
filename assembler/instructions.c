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

//error_t mem_instr(FILE* out, char* address, instr_flags flag) {
//    assert(flag == INSTR_FLAG_JMP || 
//            flag == INSTR_FLAG_CALL ||
//            flag == INSTR_FLAG_JV0 ||
//            flag == INSTR_FLAG_MOV_I);
//    int number = parse_number(address);
//    if (number >= 0x1000 || number < 0) return -1;
//    number |= flag;
//    if (write_BE(out, number) != 2) return -1;
//    return 0;
//}

error_t reg_reg_instr(FILE* out, char* op1, char* op2, instr_flags flag) {
    // checking if operands are valid registers
    regex_t regex;
    int status = regcomp(&regex, "^v([0-9]|[A-F])$", REG_EXTENDED);
    if (status != 0) return -2;
    status += regexec(&regex, op1, 0, NULL, 0);
    status += regexec(&regex, op2, 0, NULL, 0);
    regfree(&regex);
    if (status != 0) return -1;


    uint16_t reg1 = parse_digit(op1[1]);
    if (reg1 < 0 || reg1 >= 0x10) return -1;
    uint16_t reg2 = parse_digit(op2[1]);
    if (reg2 < 0 || reg2 >= 0x10) return -1;
    reg1 = ((reg1 << 8) | (reg2 << 4) | flag);
    if (write_BE(out, reg1) != 2) return -1;
    return 0;
}
