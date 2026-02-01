#include <stdio.h>
#include "parser.h"
#include <string.h>
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
    // folosite pentru construirea numelui fisierului de out 
    int out_name_len = strlen(argv[1]) + 5;
    char out_filename[out_name_len];
    strcpy(out_filename, argv[1]);
    strcat(out_filename, ".out");



    char line[256];

    // citim fisierul linie cu linie si apelam parse_line()
    while (fgets(line, sizeof(line), f)) {
        parse_line(line);
    }

    fclose(f);

    // pt Debugging: afisam rezultatul din parser la stdout
    printf("--- DEBUG PARSER ---\n");
    for (int i = 0; i < program_len; i++) {

        if (program[i].type == INSTR_MOV) {
            printf("MOV %s -> %s\n", program[i].op1, program[i].op2);
        }
        else if (program[i].type == INSTR_XOR) {
            printf("XOR %s -> %s\n", program[i].op1, program[i].op2);
        }
        else if (program[i].type == INSTR_INT) {
            printf("INT %s\n", program[i].op1);
        }
        else if (program[i].type == INSTR_INC) {
            printf("INC %s\n", program[i].op1);
        }
        else if (program[i].type == INSTR_ADD) {
            printf("ADD %s, %s\n", program[i].op1, program[i].op2);
        }
        else if (program[i].type == INSTR_SUB) {
            printf("SUB %s, %s\n", program[i].op1, program[i].op2);
        }
        else if (program[i].type == INSTR_CMP) {
            printf("CMP %s, %s\n", program[i].op1, program[i].op2);
        }
    }
    printf("--------------------\n");
    FILE *out = fopen(out_filename, "w");
    emit_program(out);
    fclose(out);

    return 0;
}
