#include <stdio.h>
#include <string.h>
#include "parser.h"

// definim vectorul extern declarat în parser.h
struct Instruction program[MAX_INSTR];
int program_len = 0;

char data_section[MAX_DATA];
int data_len = 0;
int in_data = 0;

// sterge newline de la finalul liniei
void trim(char *s) {
    int i = 0;
    while (s[i] != '\0') {
        if (s[i] == '\n' || s[i] == '\r') {
            s[i] = '\0';
            break;
        }
        i++;
    }
}

void parse_line(char *line) {
    trim(line);
    
    // eliminam spatiile si tab-urile de la inceputul liniei
    while (line[0] == ' ' || line[0] == '\t') {
        memmove(line, line + 1, strlen(line));
    }

    // normalizam movl -> mov (ca sa il putem parsa usor, oricum nu ne trebuie tipul in instructiune neaparat)
    if (strncmp(line, "movl", 4) == 0) {
        // mutam stringul cu o pozitie la stanga
        memmove(line + 3, line + 4, strlen(line + 4) + 1);
        line[3] = ' ';   // obtinem "mov ..."
    }

    // daca linie goala
    if (line[0] == 0)
        return;

    // daca .data
    if (strstr(line, ".data")) {
        in_data = 1;
        return;
    }

    // daca .text
    if (strstr(line, ".text")) {
        in_data = 0;
        return;
    }

    // daca suntem in .data, copiem tot exact cum este
    if (in_data) {
        int len = strlen(line);
        memcpy(data_section + data_len, line, len);
        data_len += len;
        data_section[data_len++] = '\n';
        return;
    }

    // de aici — doar cod din .text

    // directive (.global, etc.)
    if (line[0] == '.')
        return;

    // etichete in .text (main:, et_parcurgere:, etc.)
    if (strchr(line, ':')) {
        // scoatem doua puncte
        line[strcspn(line, ":")] = 0;

        program[program_len].type = INSTR_LABEL;
        strcpy(program[program_len].op1, line);
        program_len++;
        return;
    }

    char a[32], b[32];
    unsigned int interrupt_no;

    // MOV dest, src
    if (sscanf(line, "mov %31[^,], %31s", a, b) == 2) {

        // eliminam sufixul "l " din fata operandului (ex: movl $1, %eax)
        if (a[0] == 'l' && a[1] == ' ')
            memmove(a, a + 2, strlen(a + 1));

        program[program_len].type = INSTR_MOV;
        strcpy(program[program_len].op1, a);
        strcpy(program[program_len].op2, b);
        program_len++;
        return;
    }

    // XOR dest, src
    if (sscanf(line, "xor %31[^,], %31s", a, b) == 2) {
        program[program_len].type = INSTR_XOR;
        strcpy(program[program_len].op1, a);
        strcpy(program[program_len].op2, b);
        program_len++;
        return;
    }

    // INT $value
    if (sscanf(line, "int $%x", &interrupt_no) == 1) {
        program[program_len].type = INSTR_INT;
        program[program_len].value = interrupt_no;
        program_len++;
        return;
    }

    // CMP
    if (sscanf(line, "cmp %31[^,], %31s", a, b) == 2) {
        program[program_len].type = INSTR_CMP;
        strcpy(program[program_len].op1, a);
        strcpy(program[program_len].op2, b);
        program_len++;
        return;
    }

    // JGE label
    if (sscanf(line, "jge %31s", a) == 1) {
        program[program_len].type = INSTR_JGE;
        strcpy(program[program_len].op1, a);
        program_len++;
        return;
    }

    // JMP label
    if (sscanf(line, "jmp %31s", a) == 1) {
        program[program_len].type = INSTR_JMP;
        strcpy(program[program_len].op1, a);
        program_len++;
        return;
    }

}

