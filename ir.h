#ifndef IR_H
#define IR_H

enum operand_type {
    OP_REG,
    OP_IMM,
    OP_LABEL
};

enum reg {
    EAX, EBX, ECX, EDX
};

struct operand {
    enum operand_type type;
    union {
        enum reg reg;
        int imm;
        char *label;
    };
};

enum instr_type {
    I_MOV,
    I_XOR,
    I_INT
};

struct instr {
    enum instr_type type;
    struct operand dst;
    struct operand src;
};

#endif
