#include <stdio.h>
#include "parser.h"
#include <string.h>

void emit_shift_table(FILE *out) { // folosit pentru a "calma" numarul cu care va fi shiftat operandul, inainte de a folosi acel numar
    fprintf(out, "shift_table:\n");    // pentru a cauta in lookup table-ul respectiv instructiunii
    fprintf(out, "    .byte ");     // economisim astfel o tona de memorie fata de celelalte lookup table-uri
    for (int i = 0; i < 256; i++) { // de la 256 * 256 la 256 * 8 => de la 64kib la 2kib => eficienta!
        int index = (i < 8) ? i : 8; // orice peste 8 va deveni 8, deoarece suntem pe 8 biti 
        fprintf(out, "%d%s", index, (i == 255) ? "" : ", ");
    }
    fprintf(out, "\n");
}

void emit_shr_table(FILE *out) {
    fprintf(out, "shr_table:\n");
    for (int i = 0; i <= 8; i++) { // orice peste 8 la row va fi mereu 0, iar indexul va fi limitat la 8 inainte de a fi accesat acest table
        fprintf(out, "    .byte ");
        for (int j = 0; j < 256; j++) { 
            int shr = (i == 8) ? 0 : (j >> i);  // again, daca i==8, va fi 100% 0, altfel numarul j shiftat cu i pozitii
            fprintf(out, "%d%s", shr, (j == 255) ? "" : ", ");
        }
        fprintf(out, "\n");
    }
}

void emit_shl_table(FILE *out) {
    fprintf(out, "shl_table:\n");
    for (int i = 0; i <= 8; i++) { // again, peste sau egal cu 8 va fi 0 
        fprintf(out, "    .byte ");
        for (int j = 0; j < 256; j++) { 
            int shl = (i == 8) ? 0 : (j << i) & 255;  // same logic ca la shr, doar ca lucram pe 8 biti asa ca vom folosi and 255 
            fprintf(out, "%d%s", shl, (j == 255) ? "" : ", ");  // pentru a evita potentiale "overflow-uri" (255 = 1111 1111 deci raman bitii 0-7 neatinsi
        }                                                       // indiferent de valoarea lor)
        fprintf(out, "\n");
    }
}   // table-ul asta va fi folosit si la sal, deoarece sunt literalmente aceeasi instructiune cu aceelasi opcode

void emit_sar_table(FILE *out) {
    fprintf(out, "sar_table:\n");
    for (int i = 0; i <= 8; i++) { // putin mai special, deoarece sar e pentru signed int si opereaza in functie de semn
        fprintf(out, "    .byte ");
        for (int j = 0; j < 256; j++) { 
            int sar;
            if (i == 8) { // deoarece am dat shift de 8 ori, numarul meu va fi numai: msb x 8 
                if (j < 128) sar = 0; // daca sunt exclusiv sub 128 => sunt pozitiv deci msb == 0 => sar = 0000 0000
                else sar = 255; // altfel sunt peste 128 inclusiv => sunt negativ deci msb == 1 => sar = 1111 1111 (-1)
            }
            else { // pentru orice alt row
                int8_t val = (int8_t)j; // voi converti in format signed pe 8 biti, ca sa pot efectua shift-ul in C, altfel ar ar ignora msb si faptul ca e signed
                sar = (val >> i) & 255; // nu pot trece de 8 biti, iar el cand face operatii converteste chiar daca aveam val pe 8 biti, deci folosesc
                                        // & 255 (1111 1111) ca sa fiu sigur ca sunt pe 8 biti 
            }
            fprintf(out, "%d%s", sar, (j == 255) ? "" : ", ");
        }
        fprintf(out, "\n");
    }
}



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

void emit_xor_table(FILE *out) {
    fprintf(out, "xor_table:\n");
    for (int i = 0; i < 256; i++) {
        fprintf(out, "    .byte ");
        for (int j = 0; j < 256; j++) {
            int xor = i ^ j;
            fprintf(out, "%d%s", xor, (j == 255) ? "" : ", ");
        }
        fprintf(out, "\n");
    }
}

void emit_and_table(FILE *out) {
    fprintf(out, "and_table:\n");
    for (int i = 0; i < 256; i++) {
        fprintf(out, "    .byte ");
        for (int j = 0; j < 256; j++) {
            int and = i & j;
            fprintf(out, "%d%s", and, (j == 255) ? "" : ", ");
        }
        fprintf(out, "\n");
    }
}

void emit_or_table(FILE *out) {
    fprintf(out, "or_table:\n");
    for (int i = 0; i < 256; i++) {
        fprintf(out, "    .byte ");
        for (int j = 0; j < 256; j++) {
            int or = i | j;
            fprintf(out, "%d%s", or, (j == 255) ? "" : ", ");
        }
        fprintf(out, "\n");
    }
}

void emit_not_table(FILE *out) {
    fprintf(out, "not_table:\n");
    fprintf(out, "    .byte ");
    for (int i = 0; i < 256; i++) {

        fprintf(out, "%d%s", (~i) & 255, (i == 255) ? "" : ", "); // & 255 pentru ca ~ inverseaza toti bitti int-ului, iar astfel ramanem in cei primii 8 biti
    }                                                             // 1111 1111 & ~i va fi ~i pe 8 biti 
    fprintf(out, "\n"); 
}

void emit_program(FILE *out) {

    // pastram .data exact cum a fost
    fprintf(out, ".data\n");
    fwrite(data_section, 1, data_len, out);
    fprintf(out, "tmp: .long 0\n");
    fprintf(out, "cmp_result: .long 0\n");

    fprintf(out, "val1: .long 0\n");
    fprintf(out, "val2: .long 0\n");
    fprintf(out, "restore: .long 0\n");

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
            // folosim lookup table-ul creat 
            else {
                fprintf(out, "    # Movfuscated XOR %s, %s\n", program[i].op1, program[i].op2);

                // Salvam operanzii in memorie inainte sa distrugem vreun registru
                fprintf(out, "    movl %s, val1\n", program[i].op1); 
                fprintf(out, "    movl %s, val2\n", program[i].op2);

                // ebx va fi salvat daca este sursa, altfel oricum este modificat ca destinatie 
                // ebx va fi curatat

                if (strcmp(program[i].op2, "%%ebx") != 0){
                    fprintf(out, "    movl %%ebx, restore\n");
                } 
                fprintf(out, "    movl $0, %%ebx\n");
                
                // Construim indexul din memorie
                // Luam byte-ul din val1 si il punem in BH (High Byte)
                fprintf(out, "    movb val1, %%bh\n");
                
                // Luam byte-ul din val2 si il punem in BL (Low Byte)
                fprintf(out, "    movb val2, %%bl\n");  
                
                // Lookup in tabel. BX = (val1 * 256) + val2
                fprintf(out, "    movzbl xor_table(%%ebx), %%ebx\n");
                
                fprintf(out, "    mov %%ebx, %s\n", program[i].op2);

                // restaurez %EBX daca nu e destination in instructiune (inst src, dest)
                if (strcmp(program[i].op2, "%%ebx") != 0) {
                    fprintf(out, "    movl restore, %%ebx\n");
                }
            }
        }

        else if (program[i].type == INSTR_AND) {

            fprintf(out, "    # Movfuscated AND %s, %s\n", program[i].op1, program[i].op2);

            // Salvam operanzii in memorie inainte sa distrugem vreun registru
            fprintf(out, "    movl %s, val1\n", program[i].op1); 
            fprintf(out, "    movl %s, val2\n", program[i].op2);

            // salvez si curat ebx
            if (strcmp(program[i].op2, "%%ebx") != 0){
                fprintf(out, "    movl %%ebx, restore\n");
            } 
            fprintf(out, "    movl $0, %%ebx\n");
            
            // Construim indexul din memorie
            // Luam byte-ul din val1 si il punem in BH (High Byte)
            fprintf(out, "    movb val1, %%bh\n");
            
            // Luam byte-ul din val2 si il punem in BL (Low Byte)
            fprintf(out, "    movb val2, %%bl\n");  
            
            // Lookup in tabel. BX = (val1 * 256) + val2
            fprintf(out, "    movzbl and_table(%%ebx), %%ebx\n");
            
            fprintf(out, "    mov %%ebx, %s\n", program[i].op2);

            // restaurez %EBX daca nu e destination in instructiune (inst src, dest)
            if (strcmp(program[i].op2, "%%ebx") != 0) {
                fprintf(out, "    movl restore, %%ebx\n");
            }
        
        }

        else if (program[i].type == INSTR_OR) {
            // la fel ca la XOR

            fprintf(out, "    # Movfuscated OR %s, %s\n", program[i].op1, program[i].op2);

            // Salvam operanzii in memorie inainte sa distrugem vreun registru
            fprintf(out, "    movl %s, val1\n", program[i].op1); 
            fprintf(out, "    movl %s, val2\n", program[i].op2);

            // ebx va fi curat
            if (strcmp(program[i].op2, "%%ebx") != 0){
                fprintf(out, "    movl %%ebx, restore\n");
            } 

            fprintf(out, "    movl $0, %%ebx\n");
            
            // Construim indexul din memorie
            // Luam byte-ul din val1 si il punem in BH (High Byte)
            fprintf(out, "    movb val1, %%bh\n");
            
            // Luam byte-ul din val2 si il punem in BL (Low Byte)
            fprintf(out, "    movb val2, %%bl\n");  
            
            // Lookup in tabel. BX = (val1 * 256) + val2
            fprintf(out, "    movzbl or_table(%%ebx), %%ebx\n");
            
            fprintf(out, "    mov %%ebx, %s\n", program[i].op2);

            // restaurez %EBX daca nu e destination in instructiune (inst src, dest)
            if (strcmp(program[i].op2, "%%ebx") != 0) {
                fprintf(out, "    movl restore, %%ebx\n");
            }
        }

        else if (program[i].type == INSTR_NOT) {

            fprintf(out, "    # Movfuscated NOT %s\n", program[i].op1);

            // Salvam operandul in memorie inainte sa il distrugem
            fprintf(out, "    movl %s, val1\n", program[i].op1); 

            // ebx va fi curat
            if (strcmp(program[i].op1, "%%ebx") != 0){
                fprintf(out, "    movl %%ebx, restore\n");
            } 

            fprintf(out, "    movl $0, %%ebx\n");
            
            // punem byte-ul din val2 in bl 
            fprintf(out, "    movb val1, %%bl\n");  
            
            // lookup table-ul de la not, doar cu bl (0-255)
            fprintf(out, "    movzbl not_table(%%ebx), %%ebx\n");
            
            fprintf(out, "    mov %%ebx, %s\n", program[i].op1);

            // restaurez %EBX
            if (strcmp(program[i].op1, "%%ebx") != 0) {
                fprintf(out, "    movl restore, %%ebx\n");
            }
            
        }   
        // SHL / SAL - shift left (unsigned int/signed int)
        // Deoarece acestea sunt aceeasi instructiune (au aceelasi opcode), implementarea va fi exact la fel 
        else if (program[i].type == INSTR_SHL || program[i].type == INSTR_SAL) {
   

            fprintf(out, "    # Movfuscated SHL / SAL %s, %s\n", program[i].op1, program[i].op2);

            // Salvam operanzii in memorie inainte sa distrugem vreun registru
            fprintf(out, "    movl %s, val1\n", program[i].op1); 
            fprintf(out, "    movl %s, val2\n", program[i].op2);
            
            // ebx va fi curat si restaurat in caz ca nu e dest
            if (strcmp(program[i].op2, "%%ebx") != 0){
                fprintf(out, "    movl %%ebx, restore\n");
            } 

            fprintf(out, "    movl $0, %%ebx\n");
            // Lookup table-ul meu va fi de 9 row-uri = > 0-8 
            // va fi necesara folosirea lookup table-ului de shift pentru a redirectiona orice peste 8 biti spre linia 9 (index 8 adica)
            // deoarece orice shift cu 8 sau mai multi biti va fi 0x00
            
            
            fprintf(out, "    movb val1, %%bl\n");
            fprintf(out, "    movzbl shift_table(%%ebx), %%ebx\n"); 
            fprintf(out, "    movl %%ebx, val1\n"); // mut in val1 si reincep implementatia clasica de pana acum - val1 va fi intre 0 si 8
            fprintf(out, "    movl $0, %%ebx\n"); // restorez ebx


            // Construim indexul din memorie
            // Luam byte-ul din val1 si il punem in BH (High Byte)
            fprintf(out, "    movb val1, %%bh\n");
            
            // Luam byte-ul din val2 si il punem in BL (Low Byte)
            fprintf(out, "    movb val2, %%bl\n");  
            
            // Lookup in tabel. BX = (val1 * 256) + val2
            fprintf(out, "    movzbl shl_table(%%ebx), %%ebx\n");
            
            fprintf(out, "    mov %%ebx, %s\n", program[i].op2);

            // restaurez %EBX daca nu e destination in instructiune (inst src, dest)
            if (strcmp(program[i].op2, "%%ebx") != 0) {
                fprintf(out, "    movl restore, %%ebx\n");
            }
        }
        // SHR
                else if (program[i].type == INSTR_SHR) {
   

            fprintf(out, "    # Movfuscated SHR %s, %s\n", program[i].op1, program[i].op2);

            // Salvam operanzii in memorie inainte sa distrugem vreun registru
            fprintf(out, "    movl %s, val1\n", program[i].op1); 
            fprintf(out, "    movl %s, val2\n", program[i].op2);
            
            // ebx va fi curat si restaurat in caz ca nu e dest
            if (strcmp(program[i].op2, "%%ebx") != 0){
                fprintf(out, "    movl %%ebx, restore\n");
            } 

            fprintf(out, "    movl $0, %%ebx\n");
            // Lookup table-ul meu va fi de 9 row-uri = > 0-8 
            // va fi necesara folosirea lookup table-ului de shift pentru a redirectiona orice peste 8 biti spre linia 9 (index 8 adica)
            // deoarece orice shift cu 8 sau mai multi biti va fi 0x00
            
            
            fprintf(out, "    movb val1, %%bl\n");
            fprintf(out, "    movzbl shift_table(%%ebx), %%ebx\n"); 
            fprintf(out, "    movl %%ebx, val1\n"); // mut in val1 si reincep implementatia clasica de pana acum - val1 va fi intre 0 si 8
            fprintf(out, "    movl $0, %%ebx\n"); // restorez ebx


            // Construim indexul din memorie
            // Luam byte-ul din val1 si il punem in BH (High Byte)
            fprintf(out, "    movb val1, %%bh\n");
            
            // Luam byte-ul din val2 si il punem in BL (Low Byte)
            fprintf(out, "    movb val2, %%bl\n");  
            
            // Lookup in tabel. BX = (val1 * 256) + val2
            fprintf(out, "    movzbl shr_table(%%ebx), %%ebx\n");
            
            fprintf(out, "    mov %%ebx, %s\n", program[i].op2);

            // restaurez %EBX daca nu e destination in instructiune (inst src, dest)
            if (strcmp(program[i].op2, "%%ebx") != 0) {
                fprintf(out, "    movl restore, %%ebx\n");
            }
        }

        // SAR

        else if (program[i].type == INSTR_SAR) {
   

            fprintf(out, "    # Movfuscated SAR %s, %s\n", program[i].op1, program[i].op2);

            // Salvam operanzii in memorie inainte sa distrugem vreun registru
            fprintf(out, "    movl %s, val1\n", program[i].op1); 
            fprintf(out, "    movl %s, val2\n", program[i].op2);
            
            // ebx va fi curat si restaurat in caz ca nu e dest
            if (strcmp(program[i].op2, "%%ebx") != 0){
                fprintf(out, "    movl %%ebx, restore\n");
            } 

            fprintf(out, "    movl $0, %%ebx\n");
            // Lookup table-ul meu va fi de 9 row-uri = > 0-8 
            // va fi necesara folosirea lookup table-ului de shift pentru a redirectiona orice peste 8 biti spre linia 9 (index 8 adica)
            // aici orice shift peste 8 va fi 1111 1111 daca e negativ sau 0000 0000 daca e pozitiv in complement fata de 2 
            // n <= 127 pozitiv, n >= 128 negativ
            
            fprintf(out, "    movb val1, %%bl\n");
            fprintf(out, "    movzbl shift_table(%%ebx), %%ebx\n"); 
            fprintf(out, "    movl %%ebx, val1\n"); // mut in val1 si reincep implementatia clasica de pana acum - val1 va fi intre 0 si 8
            fprintf(out, "    movl $0, %%ebx\n"); // restorez ebx


            // Construim indexul din memorie
            // Luam byte-ul din val1 si il punem in BH (High Byte)
            fprintf(out, "    movb val1, %%bh\n");
            
            // Luam byte-ul din val2 si il punem in BL (Low Byte)
            fprintf(out, "    movb val2, %%bl\n");  
            
            // Lookup in tabel. BX = (val1 * 256) + val2
            fprintf(out, "    movzbl sar_table(%%ebx), %%ebx\n");
            
            fprintf(out, "    mov %%ebx, %s\n", program[i].op2);

            // restaurez %EBX daca nu e destination in instructiune (inst src, dest)
            if (strcmp(program[i].op2, "%%ebx") != 0) {
                fprintf(out, "    movl restore, %%ebx\n");
            }
        }

        else if (program[i].type == INSTR_INT) {
            // Folosim %s pentru ca op1 contine textul "$0x80" direct
            fprintf(out, "    int %s\n", program[i].op1);
        }
        
        else if (program[i].type == INSTR_CMP) {
            // deocamdata nu comparam — doar salvam operanzii
            // fprintf(out, "    mov %s, tmp\n", program[i].op1);
            // fprintf(out, "    mov tmp, cmp_result\n");

            // ba ii voi salva pe ambii operatori 
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
            // era de 2 ori fprintf(out, "    # Movfuscated ADD %s, %s\n", program[i].op1, program[i].op2);
            
            // Strategia:
            // Op1 -> BH (High byte)
            // Op2 -> BL (Low byte)
            // Rezultat = add_table[BX]
            
            fprintf(out, "    # Movfuscated ADD %s, %s\n", program[i].op1, program[i].op2);
            
            // Salvam operanzii in memorie inainte sa distrugem vreun registru.
            fprintf(out, "    movl %s, val1\n", program[i].op1); 
            fprintf(out, "    movl %s, val2\n", program[i].op2);
            
            // Pregatim ebx pentru a fi index
            if (strcmp(program[i].op2, "%%ebx") != 0){
                fprintf(out, "    movl %%ebx, restore\n");
            }

            fprintf(out, "    movl $0, %%ebx\n");
            
            // Construim indexul din memorie
            // Luam byte-ul din val1 si il punem in BH (High Byte)
            fprintf(out, "    movb val1, %%bh\n");
            
            // Luam byte-ul din val2 si il punem in BL (Low Byte)
            fprintf(out, "    movb val2, %%bl\n");  
            
            // Lookup in tabel. BX = (val1 * 256) + val2
            fprintf(out, "    movzbl add_table(%%ebx), %%ebx\n");
            
            fprintf(out, "    mov %%ebx, %s\n", program[i].op2);

            // restaurez %EBX daca nu e destination in instructiune (inst src, dest)
            if (strcmp(program[i].op2, "%%ebx") != 0) {
                fprintf(out, "    movl restore, %%ebx\n");
            }
        }

        else if (program[i].type == INSTR_SUB) {
            fprintf(out, "    # Movfuscated SUB %s, %s\n", program[i].op1, program[i].op2);
            
            // 1. Safe Storage (ca la ADD)
            fprintf(out, "    movl %s, val1\n", program[i].op1); // Sursa (scazatorul)
            fprintf(out, "    movl %s, val2\n", program[i].op2); // Destinatia (descazutul)
            
            // 2. Curatam EBX
            if (strcmp(program[i].op2, "%%ebx") != 0){
                fprintf(out, "    movl %%ebx, restore\n");
            } 

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

            // restaurez %EBX daca nu e destination in instructiune (inst src, dest)
            if (strcmp(program[i].op2, "%%ebx") != 0) {
                fprintf(out, "    movl restore, %%ebx\n");
            }
        }
    }
}