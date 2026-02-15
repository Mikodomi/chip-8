#ifndef TOKENIZER_H
#define TOKENIZER_H

// max tokens per line
#define MAX_TOKENS 4

enum token_t_enum {
    // instructions
    TOKEN_CLEAR,
    TOKEN_RET,
    TOKEN_JNAT,
    TOKEN_JMP,
    TOKEN_CALL, 
    TOKEN_SKPE,
    TOKEN_SKPNE,
    TOKEN_MOV, 
    TOKEN_ADD, 
    TOKEN_OR, 
    TOKEN_AND,
    TOKEN_XOR, 
    TOKEN_SUB, 
    TOKEN_MSR, 
    TOKEN_SUBS, 
    TOKEN_MSL, 
    TOKEN_JV0, 
    TOKEN_RAND,
    TOKEN_DRAW,
    TOKEN_IN,
    TOKEN_HH5,
    TOKEN_BCD,
    TOKEN_MOVOUT,
    TOKEN_MOVIN,

    // operands
    TOKEN_REG,
    TOKEN_I,
    TOKEN_DTM,
    TOKEN_STM,
    TOKEN_CONST,

    // other
    TOKEN_NEWLINE,
    TOKEN_INVALID,
    TOKEN_EMPTY,
};

typedef enum token_t_enum token_type;

struct token_struct {
    token_type type;
    
    // value is equal to the number of the register (in case of TOKEN_REG),
    // numeric value of the constant,
    // or the address
    // otherwise
    int value;
};
typedef struct token_struct token;

int parse_digit(char digit);
int parse_number(char* word);
int tokenize_line(char* src, token* dest);

#endif
