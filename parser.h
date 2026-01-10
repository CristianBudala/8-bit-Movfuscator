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
    INSTR_INT,
    INSTR_CMP,
    INSTR_JGE,
    INSTR_JMP,
    INSTR_LABEL
};

// structura unei instructiuni
struct Instruction {
    enum InstrType type;
    char op1[32];   // operand 1 ca text (ex: "$4", "%eax", "x1")
    char op2[32];   // operand 2 ca text
    int value;      // folosit doar pentru INT (ex: 0x80)
};

// vectorul de instructiuni
extern struct Instruction program[MAX_INSTR];
extern int program_len;

// functiile exportate de parser.c
void parse_line(char *line);
void trim(char *s);

#endif
