#include "tokenizer.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <inttypes.h>
#include <regex.h>

#define SPECIAL_CHARS " ;,\n"


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
        if (digit < 0 || digit >= base) return -1;
        parsed += (parse_digit(word[i]) * mult);
        mult *= base;
    }
    return parsed;
}

void empty_toks(token* dest, int n) {
    for (int i = 0; i<n; i++) {
        dest[i].type = TOKEN_EMPTY;
        dest[i].value = -1;
    }
}

// returns number of parsed tokens
// dest must be able to hold at least MAX_TOKENS tokens
int tokenize_line(char* src, token* dest) {
    empty_toks(dest, MAX_TOKENS);
    int index = 0;
    int tokamount = 0;
    char* curr;
    curr = strtok(src, SPECIAL_CHARS);
    // determining instruction
    // this looks horrendous but i promise
    // its not that bad
    if (strcmp(curr, "clear") == 0) {
        dest[tokamount].type = TOKEN_CLEAR;
    } else if (strcmp(curr, "ret") == 0) {
        dest[tokamount].type = TOKEN_RET;
    } else if (strcmp(curr, "jnat") == 0) {
        dest[tokamount].type = TOKEN_JNAT;
    } else if (strcmp(curr, "jmp") == 0) {
        dest[tokamount].type = TOKEN_JMP;
    } else if (strcmp(curr, "call") == 0) {
        dest[tokamount].type = TOKEN_CALL;
    } else if (strcmp(curr, "skpe") == 0) {
        dest[tokamount].type = TOKEN_SKPE;
    } else if (strcmp(curr, "skpne") == 0) {
        dest[tokamount].type = TOKEN_SKPNE;
    } else if (strcmp(curr, "mov") == 0) {
        dest[tokamount].type = TOKEN_MOV;
    } else if (strcmp(curr, "add") == 0) {
        dest[tokamount].type = TOKEN_ADD;
    } else if (strcmp(curr, "or") == 0) {
        dest[tokamount].type = TOKEN_OR;
    } else if (strcmp(curr, "and") == 0) {
        dest[tokamount].type = TOKEN_AND;
    } else if (strcmp(curr, "xor") == 0) {
        dest[tokamount].type = TOKEN_XOR;
    } else if (strcmp(curr, "sub") == 0) {
        dest[tokamount].type = TOKEN_SUB;
    } else if (strcmp(curr, "msr") == 0) {
        dest[tokamount].type = TOKEN_MSR;
    } else if (strcmp(curr, "subs") == 0) {
        dest[tokamount].type = TOKEN_SUBS;
    } else if (strcmp(curr, "msl") == 0) {
        dest[tokamount].type = TOKEN_MSL;
    } else if (strcmp(curr, "jv0") == 0) {
        dest[tokamount].type = TOKEN_JV0;
    } else if (strcmp(curr, "rand") == 0) {
        dest[tokamount].type = TOKEN_RAND;
    } else if (strcmp(curr, "draw") == 0) {
        dest[tokamount].type = TOKEN_DRAW;
    } else if (strcmp(curr, "in") == 0) {
        dest[tokamount].type = TOKEN_IN;
    } else if (strcmp(curr, "hh5") == 0) {
        dest[tokamount].type = TOKEN_HH5;
    } else if (strcmp(curr, "bcd") == 0) {
        dest[tokamount].type = TOKEN_BCD;
    } else if (strcmp(curr, "movout") == 0) {
        dest[tokamount].type = TOKEN_MOVOUT;
    } else if (strcmp(curr, "movin") == 0) {
        dest[tokamount].type = TOKEN_MOVIN;
    } else {
        // labels not implemented
        dest[tokamount].type = TOKEN_INVALID;
        return 0;
    }

    while((curr = strtok(NULL, SPECIAL_CHARS)) != NULL && tokamount < MAX_TOKENS-1) {
        tokamount++;
        if (!curr) break;
        // register
        int value;
        if (curr[0] == 'v' && curr[2] == '\0') {
            value = parse_digit(curr[1]);
            if (value < 0 || value > 0xF) {
                dest[tokamount].type = TOKEN_INVALID;
                tokamount--;
                break;
            }
            dest[tokamount].type = TOKEN_REG;
            dest[tokamount].value = value;
        } else if (curr[0] == 'I') {
            dest[tokamount].type = TOKEN_I;
        } else if (strcmp(curr, "dtm") == 0) {
            dest[tokamount].type = TOKEN_DTM;
        } else if (strcmp(curr, "stm") == 0) {
            dest[tokamount].type = TOKEN_STM;
        } else if ((value = parse_number(curr)) >= 0){
            dest[tokamount].type = TOKEN_CONST;
            dest[tokamount].value = value;
        } else {
            dest[tokamount].type = TOKEN_INVALID;
            tokamount--;
            break;
        }
    } 
    return ++tokamount;
}
