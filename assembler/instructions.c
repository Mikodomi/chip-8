#include "instructions.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

int assemble(FILE* in, FILE* out) {
    char* lineptr;
    size_t len = 0;
    ssize_t nread;
    char* word;
    char* word2;
    while ((nread = getline(&lineptr, &len, in)) != -1) {
        if (lineptr[0] == ';') continue;
        word = strtok(lineptr, " ;\n");
        // 1 word instructions
        if (strcmp(word, "clear") == 0) {
            fwrite("\x00\xE0", 1, 2, out);
            continue;
        } else if (strcmp(word, "ret") == 0) {
            fwrite("\x00\xEE", 1, 2, out);
            continue;
        }
        word2 = strtok(NULL, " ;\n");
        if (strcmp(word, "jmp") == 0 ) {
            jmp_call(out, word2, INSTR_FLAG_JMP);
        } else if (strcmp(word, "call") == 0) {
            jmp_call(out, word2, INSTR_FLAG_CALL);
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

int jmp_call(FILE* out, char* address, instr_flags flag) {
    assert(flag == INSTR_FLAG_JMP || flag == INSTR_FLAG_CALL);
    int number = parse_number(address);
    if (number >= 0x1000 || number < 0) return -1;
    char res[2];
    number |= flag;
    res[0] = (number & 0xFF00) >> 8;
    res[1] = (number & 0x00FF);
    size_t wrote = fwrite(res, 1, 2, out);
    if (wrote != 2) return -1;
    return 0;
}

