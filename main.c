#include <stdio.h>
#include "parser.h"

void emit_program(FILE *out);


int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s file.s\n", argv[0]);
        return 1;
    }

    FILE *f = fopen(argv[1], "r");
    if (!f) {
        perror("fopen");
        return 1;
    }

    char line[256];

    // citim fisierul linie cu linie si apelam parse_line()
    while (fgets(line, sizeof(line), f)) {
        parse_line(line);
    }

    fclose(f);

    // pentru DEBUG: afisam rezultatul din parser la stdout
    for (int i = 0; i < program_len; i++) {

        if (program[i].type == INSTR_MOV) {
            printf("MOV %s -> %s\n", program[i].op1, program[i].op2);
        }
        else if (program[i].type == INSTR_XOR) {
            printf("XOR %s -> %s\n", program[i].op1, program[i].op2);
        }
        else if (program[i].type == INSTR_INT) {
            printf("INT %x\n", program[i].value);
        }
    }

    FILE *out = fopen("output.s", "w");
    emit_program(out);
    fclose(out);

    return 0;
}
