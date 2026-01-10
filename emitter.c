#include <stdio.h>
#include "parser.h"
#include <string.h>

void emit_program(FILE *out) {

    // pastram .data exact cum a fost
    fprintf(out, ".data\n");
    fwrite(data_section, 1, data_len, out);
    fprintf(out, "tmp: .long 0\n");
    fprintf(out, "cmp_result: .long 0\n");

    fprintf(out, ".text\n");
    fprintf(out, ".global main\n");
    fprintf(out, "main:\n");


    for (int i = 0; i < program_len; i++) {

        if (program[i].type == INSTR_MOV) {

            // daca sursa e constanta ($...)
            if (program[i].op1[0] == '$') {

                fprintf(out, "    mov %s, tmp\n", program[i].op1);
                fprintf(out, "    mov tmp, %s\n", program[i].op2);
            }
            else {
                // altfel — scriem mov normal
                fprintf(out, "    mov %s, %s\n",
                        program[i].op1,
                        program[i].op2);
            }
        }

        else if (program[i].type == INSTR_XOR) {

            // daca avem xor %reg, %reg
            if (strcmp(program[i].op1, program[i].op2) == 0) {
                fprintf(out, "    mov $0, %s\n", program[i].op1);
            }
            else {
                // alt xor (pe viitor) – il lasam deocamdata
                fprintf(out, "    xor %s, %s\n",
                        program[i].op1,
                        program[i].op2);
            }
        }

        else if (program[i].type == INSTR_INT) {
            fprintf(out, "    int $0x%x\n",
                    program[i].value);
        }
        

        else if (program[i].type == INSTR_CMP) {
            // deocamdata nu comparam — doar salvam operanzii
            fprintf(out, "    mov %s, tmp\n", program[i].op1);
            fprintf(out, "    mov tmp, cmp_result\n");
        }

        else if (program[i].type == INSTR_JGE) {
            fprintf(out, "    jge %s\n", program[i].op1);
        }

        else if (program[i].type == INSTR_JMP) {
            fprintf(out, "    jmp %s\n", program[i].op1);
        }

        else if (program[i].type == INSTR_LABEL) {
            fprintf(out, "%s:\n", program[i].op1);
        }
    }
}
