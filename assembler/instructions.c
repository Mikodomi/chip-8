#include "instructions.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <inttypes.h>

#define SPECIAL_CHARS " ;,\n"

int assemble(FILE* in, FILE* out) {
    char* lineptr;
    size_t len = 0;
    ssize_t nread;
    char* instr;
    char *op1, *op2, *op3;
    while ((nread = getline(&lineptr, &len, in)) != -1) {
        if (lineptr[0] == ';' || lineptr[0] == '\n') continue;
        instr = strtok(lineptr, SPECIAL_CHARS);
        if (!instr) break;
        // instructions with no operand
        if (strcmp(instr, "clear") == 0) {
            fwrite("\x00\xE0", 1, 2, out);
            continue;
        } else if (strcmp(instr, "ret") == 0) {
            fwrite("\x00\xEE", 1, 2, out);
            continue;
        }
        // 1 operand instructions
        op1 = strtok(NULL, SPECIAL_CHARS);
        if (strcmp(instr, "jmp") == 0 ) {
            mem_instr(out, op1, INSTR_FLAG_JMP);
            continue;
        } else if (strcmp(instr, "call") == 0) {
            mem_instr(out, op1, INSTR_FLAG_CALL);
            continue;
        } else if (strcmp(instr, "jv0") == 0) {
            mem_instr(out, op1, INSTR_FLAG_JV0);
            continue;
        }
        // 2 operand instructions
        op2 = strtok(NULL, SPECIAL_CHARS);
        // there has to be a better way to do this
        if (strcmp(instr, "mov") == 0) {
            mov(out, op1, op2);
            continue;
        } else if (strcmp(instr, "or") == 0) {
            reg_reg_instr(out, op1, op2, INSTR_FLAG_REG_REG_OR);
            continue;
        } else if (strcmp(instr, "and") == 0) {
            reg_reg_instr(out, op1, op2, INSTR_FLAG_REG_REG_AND);
            continue;
        } else if (strcmp(instr, "xor") == 0) {
            reg_reg_instr(out, op1, op2, INSTR_FLAG_REG_REG_XOR);
            continue;
        } else if (strcmp(instr, "add") == 0) {
            reg_reg_instr(out, op1, op2, INSTR_FLAG_REG_REG_ADD);
            continue;
        } else if (strcmp(instr, "sub") == 0) {
            reg_reg_instr(out, op1, op2, INSTR_FLAG_REG_REG_SUB);
            continue;
        } else if (strcmp(instr, "msr") == 0) {
            reg_reg_instr(out, op1, op2, INSTR_FLAG_REG_REG_MSR);
            continue;
        } else if (strcmp(instr, "subs") == 0) {
            reg_reg_instr(out, op1, op2, INSTR_FLAG_REG_REG_SUBS);
            continue;
        } else if (strcmp(instr, "msl") == 0) {
            reg_reg_instr(out, op1, op2, INSTR_FLAG_REG_REG_MSL);
            continue;
        }
        
    }
    free(lineptr);
    return 0;
}

int parse_digit(char digit) {
    if (digit >= '0' && digit <= '9') {
        return digit-'0';
    }
    if (digit >= 'A' && digit <= 'F') {
        return digit-'A'+10;
    }
    return -1;
}

int parse_number(char* word) {
    int len = strlen(word);
    int base;
    int parsed = 0;
    switch (word[len-1]) {
        case 'B':
            base = 2;
            len--;
            break;
        case 'O':
            base = 8;
            len--;
            break;
        case 'H':
            base = 16;
            len--;
            break;
        default:
            base = 10;
            break;
    }
    int digit, mult = 1;
    for (int i = len-1; i>=0; i--) {
        digit = parse_digit(word[i]);
        if (digit < 0) return -1;
        parsed += (parse_digit(word[i]) * mult);
        mult *= base;
    }
    return parsed;
}

size_t write_BE(FILE* out, uint16_t in) {
    char temp[2];
    temp[0] = (in & 0xFF00) >> 8;
    temp[1] = (in & 0x00FF);
    return fwrite(temp, 1, 2, out);
}

int mem_instr(FILE* out, char* address, instr_flags flag) {
    assert(flag == INSTR_FLAG_JMP || 
            flag == INSTR_FLAG_CALL ||
            flag == INSTR_FLAG_JV0 ||
            flag == INSTR_FLAG_MOV_I);
    int number = parse_number(address);
    if (number >= 0x1000 || number < 0) return -1;
    number |= flag;
    if (write_BE(out, number) != 2) return -1;
    return 0;
}

int reg_reg_instr(FILE* out, char* op1, char* op2, instr_flags flag) {
    uint16_t reg1 = parse_digit(op1[1]);
    if (reg1 < 0 || reg1 >= 0x10) return -1;
    uint16_t reg2 = parse_digit(op2[1]);
    if (reg2 < 0 || reg2 >= 0x10) return -1;
    reg1 = ((reg1 << 8) | (reg2 << 4) | flag);
    if (write_BE(out, reg1) != 2) return -1;
    return 0;
}

int mov(FILE* out, char* op1, char* op2) {
    if (!op1) return -1;
    if (op1[0] == 'I') {
        return mem_instr(out, op1, INSTR_FLAG_MOV_I);
    }
    if (op1[0] == 'v' && op2[0] == 'v') {
        return reg_reg_instr(out, op1, op2, INSTR_FLAG_REG_REG_MOV); 
    }
    return -1; // 'mov dtm, ...' etc. unimplemented for now
}
