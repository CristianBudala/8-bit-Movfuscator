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
    // 1. Verificam daca linia e goala sau comentariu
    if (!line || strlen(line) < 2) return;

    // 2. Gestionam sectiunile .data si .text
    // Daca suntem in .data, activam flag-ul
    if (strstr(line, ".data")) {
        in_data = 1;
        return;
    }
    // Daca suntem in .text, il dezactivam
    if (strstr(line, ".text")) {
        in_data = 0;
        return;
    }

    // Daca suntem in sectiunea de date, copiem linia direct in bufferul nostru
    // Nu o parsam, o pastram asa cum e pentru asamblatorul final.
    if (in_data) {
        strcpy(data_section + data_len, line);
        data_len += strlen(line);
        // newline la final
        if (data_section[data_len - 1] != '\n') {
            data_section[data_len++] = '\n';
        }
        return;
    }

    // --- De aici parsam codul din .text ---

    // Ignoram alte directive care incep cu punct (ex: .global main)
    if (line[0] == '.') return;

    // Spargem linia in tokens cu strtok
    // Delimitatori: spatiu, tab (\t), virgula, newline (\n)
    char *token = strtok(line, " \t,\n");

    // Daca e linie goala
    if (!token) return;

    // 3. Verificam daca e eticheta (se termina in ':')
    int len = strlen(token);
    if (token[len - 1] == ':') {
        token[len - 1] = '\0'; // Stergem doua puncte
        
        program[program_len].type = INSTR_LABEL;
        strcpy(program[program_len].op1, token);
        program_len++;
        return;
    }

    // 4. Identificam instructiunea
    
    // MOV sau MOVL
    if (strcmp(token, "mov") == 0 || strcmp(token, "movl") == 0) {
        program[program_len].type = INSTR_MOV;
        
        // Luam primul operand (sursa)
        char *op1 = strtok(NULL, " \t,\n");
        // Luam al doilea operand (destinatia)
        char *op2 = strtok(NULL, " \t,\n");

        if (op1 && op2) {
            strcpy(program[program_len].op1, op1);
            strcpy(program[program_len].op2, op2);
            program_len++;
        }
    }
    // AND
    else if (strcmp(token, "and") == 0) {
        program[program_len].type = INSTR_AND;
        
        char *op1 = strtok(NULL, " \t,\n");
        char *op2 = strtok(NULL, " \t,\n");

        if (op1 && op2) {
            strcpy(program[program_len].op1, op1);
            strcpy(program[program_len].op2, op2);
            program_len++;
        }
    }
    //OR
    else if (strcmp(token, "or") == 0) {
        program[program_len].type = INSTR_OR;
        
        char *op1 = strtok(NULL, " \t,\n");
        char *op2 = strtok(NULL, " \t,\n");

        if (op1 && op2) {
            strcpy(program[program_len].op1, op1);
            strcpy(program[program_len].op2, op2);
            program_len++;
        }
    }
    // NOT
    else if (strcmp(token, "not") == 0) {
        program[program_len].type = INSTR_NOT;
        
        char *op1 = strtok(NULL, " \t,\n");
        char *op2 = strtok(NULL, " \t,\n");

        if (op1 && op2) {
            strcpy(program[program_len].op1, op1);
            strcpy(program[program_len].op2, op2);
            program_len++;
        }
    }
    // XOR
    else if (strcmp(token, "xor") == 0) {
        program[program_len].type = INSTR_XOR;
        
        char *op1 = strtok(NULL, " \t,\n");
        char *op2 = strtok(NULL, " \t,\n");

        if (op1 && op2) {
            strcpy(program[program_len].op1, op1);
            strcpy(program[program_len].op2, op2);
            program_len++;
        }
    }
    // CMP
    else if (strcmp(token, "cmp") == 0) {
        program[program_len].type = INSTR_CMP;
        
        char *op1 = strtok(NULL, " \t,\n");
        char *op2 = strtok(NULL, " \t,\n");

        if (op1 && op2) {
            strcpy(program[program_len].op1, op1);
            strcpy(program[program_len].op2, op2);
            program_len++;
        }
    }
    // INT (Interrupt)
    else if (strcmp(token, "int") == 0) {
        program[program_len].type = INSTR_INT;
        
        char *val = strtok(NULL, " \t,\n");
        
        if (val) {
            strcpy(program[program_len].op1, val);
            program_len++;
        }
    }
    // JGE (Jump if Greater or Equal)
    else if (strcmp(token, "jge") == 0) {
        program[program_len].type = INSTR_JGE;
        char *label = strtok(NULL, " \t,\n");
        if (label) {
            strcpy(program[program_len].op1, label);
            program_len++;
        }
    }
    // JG (Jump if greater)
    else if (strcmp(token, "jg") == 0) {
        program[program_len].type = INSTR_JG;
        char *label = strtok(NULL, " \t,\n");
        if (label) {
            strcpy(program[program_len].op1, label);
            program_len++;
        }
    }
    // JLE (Jump if less or equal)
    else if (strcmp(token, "jle") == 0) {
        program[program_len].type = INSTR_JLE;
        char *label = strtok(NULL, " \t,\n");
        if (label) {
            strcpy(program[program_len].op1, label);
            program_len++;
        }
    }
    // JL (Jump if less)
    else if (strcmp(token, "jl") == 0) {
        program[program_len].type = INSTR_JG;
        char *label = strtok(NULL, " \t,\n");
        if (label) {
            strcpy(program[program_len].op1, label);
            program_len++;
        }
    }
    // JE (Jump if equal)
    else if (strcmp(token, "je") == 0) {
        program[program_len].type = INSTR_JE;
        char *label = strtok(NULL, " \t,\n");
        if (label) {
            strcpy(program[program_len].op1, label);
            program_len++;
        }
    }
    // JNE (Jump if not equal)
    else if (strcmp(token, "jne") == 0) {
        program[program_len].type = INSTR_JNE;
        char *label = strtok(NULL, " \t,\n");
        if (label) {
            strcpy(program[program_len].op1, label);
            program_len++;
        }
    }
    // SHR (Shift Right - unsigned)
    else if (strcmp(token, "shr") == 0) {
        program[program_len].type = INSTR_SHR;
        
        char *op1 = strtok(NULL, " \t,\n");
        char *op2 = strtok(NULL, " \t,\n");

        if (op1 && op2) {
            strcpy(program[program_len].op1, op1);
            strcpy(program[program_len].op2, op2);
            program_len++;
        }
    }   
    // SAR (Shift Right - signed)
    else if (strcmp(token, "sar") == 0) {
            program[program_len].type = INSTR_SAR;
            
            char *op1 = strtok(NULL, " \t,\n");
            char *op2 = strtok(NULL, " \t,\n");

            if (op1 && op2) {
                strcpy(program[program_len].op1, op1);
                strcpy(program[program_len].op2, op2);
                program_len++;
            }
        }   
    // SHL (Shift left - unsigned)
    else if (strcmp(token, "shl") == 0) {
            program[program_len].type = INSTR_SHL;
            
            char *op1 = strtok(NULL, " \t,\n");
            char *op2 = strtok(NULL, " \t,\n");

            if (op1 && op2) {
                strcpy(program[program_len].op1, op1);
                strcpy(program[program_len].op2, op2);
                program_len++;
            }
        }   
    // SAL (Shift left - signed)
    else if (strcmp(token, "sal") == 0) {
            program[program_len].type = INSTR_SAL;
            
            char *op1 = strtok(NULL, " \t,\n");
            char *op2 = strtok(NULL, " \t,\n");

            if (op1 && op2) {
                strcpy(program[program_len].op1, op1);
                strcpy(program[program_len].op2, op2);
                program_len++;
            }
        }   
    // JMP (Jump neconditionat)
    else if (strcmp(token, "jmp") == 0) {
        program[program_len].type = INSTR_JMP;
        char *label = strtok(NULL, " \t,\n");
        if (label) {
            strcpy(program[program_len].op1, label);
            program_len++;
        }
    }
    // INC (Increment)
    else if (strcmp(token, "inc") == 0) {
        program[program_len].type = INSTR_INC;
        
        // Luam operandul (ex: %eax)
        char *op1 = strtok(NULL, " \t,\n");
        
        if (op1) {
            strcpy(program[program_len].op1, op1);
            program_len++;
        }
    }

    // ADD
    else if (strcmp(token, "add") == 0) {
        program[program_len].type = INSTR_ADD;
        
        char *op1 = strtok(NULL, " \t,\n");
        char *op2 = strtok(NULL, " \t,\n");

        if (op1 && op2) {
            strcpy(program[program_len].op1, op1);
            strcpy(program[program_len].op2, op2);
            program_len++;
        }
    }

    // SUB
    else if (strcmp(token, "sub") == 0) {
        program[program_len].type = INSTR_SUB;
        
        char *op1 = strtok(NULL, " \t,\n");
        char *op2 = strtok(NULL, " \t,\n");

        if (op1 && op2) {
            strcpy(program[program_len].op1, op1);
            strcpy(program[program_len].op2, op2);
            program_len++;
        }
    }

}

