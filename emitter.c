#include <stdio.h>
#include "parser.h"
#include <string.h>

void emit_inc_table(FILE *out) {
    fprintf(out, "inc_table:\n");
    fprintf(out, "    .byte ");
    for (int i = 0; i < 256; i++) {
        // Scriem (i + 1) % 256
        fprintf(out, "%d%s", (i + 1) % 256, (i == 255) ? "" : ", ");
    }
    fprintf(out, "\n");
}

void emit_add_table(FILE *out) {
    fprintf(out, "add_table:\n");
    // Tabelul e mare (256 * 256 bytes)
    for (int i = 0; i < 256; i++) {
        fprintf(out, "    .byte "); // Rand nou pentru fiecare i (high byte)
        for (int j = 0; j < 256; j++) {
            // Calculam suma (cu overflow natural modulo 256)
            int sum = (i + j) % 256;
            fprintf(out, "%d%s", sum, (j == 255) ? "" : ", ");
        }
        fprintf(out, "\n");
    }
}

void emit_sub_table(FILE *out) {
    fprintf(out, "sub_table:\n");
    for (int i = 0; i < 256; i++) {
        fprintf(out, "    .byte ");
        for (int j = 0; j < 256; j++) {
            // i este descazutul, j este scazatorul
            // Adaugam 256 inainte de modulo pentru a gestiona numerele negative in C
            int diff = (i - j + 256) % 256;
            fprintf(out, "%d%s", diff, (j == 255) ? "" : ", ");
        }
        fprintf(out, "\n");
    }
}

void emit_program(FILE *out) {

    // pastram .data exact cum a fost
    fprintf(out, ".data\n");
    fwrite(data_section, 1, data_len, out);
    fprintf(out, "tmp: .long 0\n");
    fprintf(out, "cmp_result: .long 0\n");

    fprintf(out, "val1: .long 0\n");
    fprintf(out, "val2: .long 0\n");

    emit_inc_table(out);
    emit_add_table(out);
    emit_sub_table(out);

    fprintf(out, ".text\n");
    fprintf(out, ".global main\n");
    fprintf(out, "main:\n");


    for (int i = 0; i < program_len; i++) {

        if (program[i].type == INSTR_MOV) {

            // daca sursa e constanta ($...)
            if (program[i].op1[0] == '$') {

                fprintf(out, "    movl %s, tmp\n", program[i].op1);
                fprintf(out, "    movl tmp, %s\n", program[i].op2);
            }
            else {
                // altfel — scriem mov normal
                fprintf(out, "    movl %s, %s\n",
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
            // Folosim %s pentru ca op1 contine textul "$0x80" direct
            fprintf(out, "    int %s\n", program[i].op1);
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

        else if (program[i].type == INSTR_INC) {
            // inc %eax => %eax = inc_table[%eax]

            fprintf(out, "    # Movfuscated INC\n");
            fprintf(out, "    movl $0, %%ebx\n");       // Curățăm ebx (ca să folosim bl)
            fprintf(out, "    mov %s, %%ebx\n", program[i].op1); // Punem valoarea în ebx
            
            // Luam rezultatul din tabel: inc_table + index
            fprintf(out, "    movzbl inc_table(%%ebx), %%ebx\n"); 
            
            // Scriem rezultatul înapoi
            fprintf(out, "    mov %%ebx, %s\n", program[i].op1);
        }

        else if (program[i].type == INSTR_ADD) {
            fprintf(out, "    # Movfuscated ADD %s, %s\n", program[i].op1, program[i].op2);
            
            // Strategia:
            // Op1 -> BH (High byte)
            // Op2 -> BL (Low byte)
            // Rezultat = add_table[BX]
            
            fprintf(out, "    # Movfuscated ADD %s, %s\n", program[i].op1, program[i].op2);
            
            // Salvam operanzii in memorie inainte sa distrugem vreun registru.
            fprintf(out, "    movl %s, val1\n", program[i].op1); 
            fprintf(out, "    movl %s, val2\n", program[i].op2);
            
            // Pregatim ebx pentru a fi index
            fprintf(out, "    movl $0, %%ebx\n");
            
            // Construim indexul din memorie
            // Luam byte-ul din val1 si il punem in BH (High Byte)
            fprintf(out, "    movb val1, %%bh\n");
            
            // Luam byte-ul din val2 si il punem in BL (Low Byte)
            fprintf(out, "    movb val2, %%bl\n");
            
            // Lookup in tabel. BX = (val1 * 256) + val2
            fprintf(out, "    movzbl add_table(%%ebx), %%ebx\n");
            
            fprintf(out, "    mov %%ebx, %s\n", program[i].op2);
        }

        else if (program[i].type == INSTR_SUB) {
            fprintf(out, "    # Movfuscated SUB %s, %s\n", program[i].op1, program[i].op2);
            
            // 1. Safe Storage (ca la ADD)
            fprintf(out, "    movl %s, val1\n", program[i].op1); // Sursa (scazatorul)
            fprintf(out, "    movl %s, val2\n", program[i].op2); // Destinatia (descazutul)
            
            // 2. Curatam EBX
            fprintf(out, "    movl $0, %%ebx\n");
            
            // 3. Setam indexul: Vrem Destinatie - Sursa
            // Deci Destinatia (val2) merge in BH (Row)
            fprintf(out, "    movb val2, %%bh\n");
            
            // Sursa (val1) merge in BL (Col)
            fprintf(out, "    movb val1, %%bl\n");
            
            // 4. Lookup sub_table[dest][src]
            fprintf(out, "    movzbl sub_table(%%ebx), %%ebx\n");
            
            // 5. Scriem rezultatul
            fprintf(out, "    mov %%ebx, %s\n", program[i].op2);
        }
    }
}