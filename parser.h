#ifndef PARSER_H
#define PARSER_H

#define MAX_INSTR 1024
#define MAX_DATA 2048

extern char data_section[MAX_DATA];
extern int data_len;

// tipurile de instructiuni
enum InstrType {
    INSTR_MOV,
    INSTR_XOR,
    INSTR_AND,
    INSTR_OR,
    INSTR_NOT,
    INSTR_INT,
    INSTR_CMP,
    INSTR_JGE,
    INSTR_JG,
    INSTR_JLE,
    INSTR_JL,
    INSTR_JE,
    INSTR_JNE,
    INSTR_LOOP,
    INSTR_PUSH,
    INSTR_POP,
    INSTR_DEC,
    INSTR_SHR,
    INSTR_SAR,
    INSTR_SHL,
    INSTR_SAL,
    INSTR_JMP,
    INSTR_LABEL,
    INSTR_INC,
    INSTR_ADD,
    INSTR_SUB,
};

// structura unei instructiuni
struct Instruction {
    enum InstrType type;
    char op1[32];   // operand 1 ca text (ex: "$4", "%eax", "x1")
    char op2[32];   // operand 2 ca text
};

// vectorul de instructiuni
extern struct Instruction program[MAX_INSTR];
extern int program_len;

// functiile exportate de parser.c
void parse_line(char *line);
void trim(char *s);

#endif
