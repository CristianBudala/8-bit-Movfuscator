#include <stdio.h>
#include "parser.h"
#include <string.h>


void emit_jge_table(FILE *out) {
    fprintf(out, "jge_table:\n");
    fprintf(out, "    .byte ");
    // scriu pur si simplu tot table-ul intr-un fprintf
    // o sa fie 8 in total si se acceseaza cu valoarea flag-ului ca offset 
    // pentru ca flag va acoperi toate valorile pe 3 biti (0-7) 

    // 000, 011, 100 vor fi valorile pt flag la care vom efectua jump-ul conditionat
    // valoarea va fi 4 (32 de biti cat are adresa unui label), deoarece cand voi folosi array-ul cu cele 2 adrese (instr ce urmeaza dupa condjump si labelul la care se face condjump),
    // voi folosi ce gasesc in table-ul asta pe post de offset 

    fprintf(out, "4,0,0,4,4,0,0,0\n");
}

void emit_jg_table(FILE *out) {
    fprintf(out, "jg_table:\n");
    fprintf(out, "    .byte ");
    // la fel ca la jge

    // 000, 011  vor fi valorile pt flag la care vom efectua jump-ul conditionat
    fprintf(out, "4,0,0,4,0,0,0,0\n");
}

void emit_jl_table(FILE *out) {
    fprintf(out, "jl_table:\n");
    fprintf(out, "    .byte ");
    // la fel ca la jge

    // 001, 010  vor fi valorile pt flag la care vom efectua jump-ul conditionat
    fprintf(out, "0,4,4,0,0,0,0,0\n");
}

void emit_jle_table(FILE *out) {
    fprintf(out, "jle_table:\n");
    fprintf(out, "    .byte ");
    // la fel ca la jge

    // 001, 010, 100  vor fi valorile pt flag la care vom efectua jump-ul conditionat
    fprintf(out, "0,4,4,0,4,0,0,0\n");
}

void emit_je_table(FILE *out) {
    fprintf(out, "je_table:\n");
    fprintf(out, "    .byte ");
    // la fel ca la jge

    // 100  va fi singura valoare valida pt flag la care vom efectua jump-ul conditionat
    fprintf(out, "0,0,0,0,4,0,0,0\n");
}

void emit_jne_table(FILE *out) {
    fprintf(out, "jne_table:\n");
    fprintf(out, "    .byte ");
    // la fel ca la jge

    // orice inafara de 100 vor fi valori pt flag la care vom efectua jump-ul conditionat
    fprintf(out, "4,4,4,4,0,4,4,4\n");
}

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
// folosita pentru a returna rezultatul unui compare, mergand pe structura zero sign overflow (va merge deci pentru orice jump-uri conditionate cu int-uri signed)
// vom structura elementele table-ului astfel:
// 0000 0000 0000 0000 0000 0000 0000 0111
// unde 1           1           1           => ultimii 3 biti
//   zeroflag    signflag   overflowflag
//
// Cmp source destination merge pe logica dest - source. => actualizare flag-uri
// Daca am cmp %eax, %ebx => ebx - eax si actualizeaza flag-urile coresp
// JG: doar daca ebx - eax > 0 <=> daca ebx > eax => deci daca am 000/011
// JGE: daca am ebx - eax >= 0 ⇔ ebx >= eax => deci pot avea 000/011/100/
// JL: daca am ebx - eax < 0 <=> ebx < eax => deci daca am 001/010
// JLE: daca am ebx-eax <= 0 <=> ebx <= eax => deci daca am 001/010/100
// JE: daca am ebx - eax == 0 <=> ebx == eax => deci daca am 100
// JNE: daca am ebx - eax != 0 <=> ebx != eax => deci daca am 000/001/010/011


// tot 256 x 256, consider i dest, j src 
// efectuez i - j si in functie de rezultat setez "flag-urile", iar la final le combin 
// zero flag daca diff e zero
// signflag dupa MSB 
// overflow va merge dupa logica:
    // extrag MSBii src, dest si diff 
    // daca scad doua numere, ambele pozitive/negative, nu pot avea overflow. Ma aproprii mereu de 0 dar nu pot trece limitele reprezentarii signed pe 8 biti 
    // ex: 127 - 0 va fi 127, 127 - 127 va fi 0     // -127 - (-1) = -126, -127 - (-127) = 0
    // asadar, daca sign-ul este aceelasi intre src si dest, 100% nu am overflow
    // altfel e posibil overflow, deoarece "cresc" intr-o anumita directie, inspre o anumita limita, nu inspre 0 ca in celelalt caz 
    // verific astfel, daca semnul lui dest este diferit de semnul rezultatului scaderii dest - src, deoarece ar inseamna ca am trecut pe cealalta parte (de la 127 la -128 de ex)




void emit_cmp_table(FILE *out) {
    fprintf(out, "cmp_table:\n");
    // 256 x 256 bytes pentru a putea compara orice numar cu orice alt numar
    for (int i = 0; i < 256; i++) {
        fprintf(out, "    .byte "); // Rand nou pentru fiecare i (high byte)
        for (int j = 0; j < 256; j++) {
            int flag = 0;
            int diff = (i - j) & 255; // 255 e 1111 1111 si simulez astfel cum ar functiona overflow-ul pe 8 biti
            int zeroflag = (diff == 0) ? 1 : 0; 
            // pentru signflag, voi folosi & 1000 0000 (128), pentru a verifica MSB 
            int signflag = (diff & 128) ? 1 : 0;
            // la fel si pentru semnele src si dest, necesare pentru determinarea unui overflow 
            int signi = (i & 128) ? 1 : 0;
            int signj = (j & 128) ? 1 : 0;

            int overflow = 0;
            if (signi != signj) { // daca am semnele egale intre dest si src, nu pot avea overfow
                if (signi != signflag) { 
                    overflow = 1;
                }
            }
            
            // profit de codificarea pe biti si adaug puterea lui 2 coresp bitului ce reprezinta flag-ul respectiv
            if (zeroflag) { 
                flag += 4; // 100
            }
            if (signflag) { 
                flag += 2; // 010
            }
            if (overflow) { 
                flag += 1; // 001
            }
            
            fprintf(out, "%d%s", flag, (j == 255) ? "" : ", ");
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

void emit_dec_table(FILE *out) {
    fprintf(out, "dec_table:\n");
    fprintf(out, "    .byte ");
    for (int i = 0; i < 256; i++) {
        fprintf(out, "%d%s", (i - 1) & 255, (i == 255) ? "" : ", ");
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
    int condjumps = 0; // va fi folosit pentru crearea de label-uri noi pentru jumpuri conditionale 
    // pastram .data exact cum a fost
    fprintf(out, ".data\n");
    fwrite(data_section, 1, data_len, out);
    fprintf(out, "tmp: .long 0\n");
    fprintf(out, "val1: .long 0\n");
    fprintf(out, "val2: .long 0\n");
    fprintf(out, "restore: .long 0\n"); // pentru restaurarea lui ebx 
    fprintf(out, "flags: .long 0\n"); 
    fprintf(out, "jumps: .long 0, 0\n"); // jumps[0] => adresa imediat dupa condjmp, jumps[1] => adresa la care sar daca cond e indeplinita 
    fprintf(out, "jumpaddress: .long 0\n"); // unde voi stoca efectiv adresa selectata din array-ul precedent
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
            // obiectivul va fi sa salvez in variabila flags (din .data) rezultatul operatiei de cmp (sub formatul tabelului, imp fiind ultimii 3 biti)
            // in lookup table, i e dest, j e src, deci 
            fprintf(out, "    # Movfuscated CMP %s, %s\n", program[i].op1, program[i].op2);
            // op2 va fi dest, op1 va fi src 
            // cmp face dest - src deci op2 - op1 = > op2 va fi i, op1 va fi j = > op1 va fi coloana, op2 va fi linia
            // deci op1 in bl, op2 in bh
            // salvez in val1, val2 operanzii
            fprintf(out, "    movl %s, val1\n", program[i].op1); // src - j
            fprintf(out, "    movl %s, val2\n", program[i].op2); // dest - i
            // salvez ebx si il curat
            fprintf(out, "    movl %%ebx, restore\n");
            fprintf(out, "    movl $0, %%ebx\n");;

            // folosesc bx pentru accesarea adresei corespunzatoare, linia in bh (*256) si coloana in bl 
            fprintf(out, "    movb val1, %%bl\n");
            fprintf(out, "    movb val2, %%bh\n");
            fprintf(out, "    movzbl cmp_table(%%ebx), %%ebx\n");
            fprintf(out, "    movl %%ebx, flags\n");

            // in flags voi avea 0000 0000 0000 0000 0000 0000 0000 0ZSO (Z - zero, S - sign, O - overflow)
            // interpretarea lui flags o sa o fac propriuzis in blocul pentru instructiunea condtionala respectiva

            // restaurez ebx 
            fprintf(out, "    movl restore, %%ebx\n");

        }
            // jump-uri conditionale
            // variabila flags din .data va contine deja rezultatul (flag-urile actualizate) instructiunii cmp
            // pe formatul 0000 0000 0000 0000 0000 0000 0000 0ZSO
            // JGE eax, ebx: daca am ebx - eax >= 0 <=> ebx >= eax => deci daca am in flags 000 / 011 / 100 sar

            // fac table-uri pentru fiecare instr conditionala 
            // structurez cu cate 7 elemente fiecare tabel - avand codificarea pe 3 biti ca cheie de accesare
            // si in ea va fi 1 sau 0 
            //                  000  001    011     111     010     110    100      101
            // .long jge_table: 1     0      1       0       0       0       1

            // ii dau codificarea (long-ul cu cei 3 biti la inceput) ca offset iar aici voi gasi fie offsetul folosit
            // pentru array-ul cu adresele celor 2 label-uri, una imediat dupa condjump la offset 0 
            // si la offset 4, label-ul la care duce condjump daca e adevarat
            

        else if (program[i].type == INSTR_JGE) {
            fprintf(out, "    # Movfuscated JGE %s\n", program[i].op1);

            // salvez si curat ebx 
            fprintf(out, "    movl %%ebx, restore\n");
            fprintf(out, "    movl $0, %%ebx\n");

            // mut adresa label-ului din op1
            fprintf(out, "    movl $%s, %%ebx\n", program[i].op1);
            fprintf(out, "    movl %%ebx, jumps+4\n");
            // mut adresa labelului "fictiv" la care sar daca cmp nu a produs flag-ul necesar sariturii
            fprintf(out, "    movl $label%d, %%ebx\n", condjumps);
            fprintf(out, "    movl %%ebx, jumps\n");
            // mut flags in ebx ca sa il folosesc pe post de offset cand caut in table-ul coresp instructiunii conditionale
            fprintf(out, "    movl flags, %%ebx\n");
            fprintf(out, "    movzbl jge_table(%%ebx), %%ebx\n");
            // mut adresa label-ului ales in jumpaddress
            fprintf(out, "    movl jumps(%%ebx), %%ebx\n");
            fprintf(out, "    movl %%ebx, jumpaddress\n");

            // restaurez ebx
            fprintf(out, "    movl restore, %%ebx\n");
            
            // * e ca sa sar la adresa din variabila
            fprintf(out, "    jmp *jumpaddress\n");

            // printez label-ul folosit in caz ca nu se executa condjump-ul 
            fprintf(out, "label%d:\n", condjumps);
            // incrementez sufixul label-urilor
            condjumps++; 
        }

        else if (program[i].type == INSTR_JG) {
            fprintf(out, "    # Movfuscated JG %s\n", program[i].op1);

            // salvez si curat ebx 
            fprintf(out, "    movl %%ebx, restore\n");
            fprintf(out, "    movl $0, %%ebx\n");

            // mut adresa label-ului din op1
            fprintf(out, "    movl $%s, %%ebx\n", program[i].op1);
            fprintf(out, "    movl %%ebx, jumps+4\n");
            // mut adresa labelului "fictiv" la care sar daca cmp nu a produs flag-ul necesar sariturii
            fprintf(out, "    movl $label%d, %%ebx\n", condjumps);
            fprintf(out, "    movl %%ebx, jumps\n");
            // mut flags in ebx ca sa il folosesc pe post de offset cand caut in table-ul coresp instructiunii conditionale
            fprintf(out, "    movl flags, %%ebx\n");
            fprintf(out, "    movzbl jg_table(%%ebx), %%ebx\n");
            // mut adresa label-ului ales in jumpaddress
            fprintf(out, "    movl jumps(%%ebx), %%ebx\n");
            fprintf(out, "    movl %%ebx, jumpaddress\n");

            // restaurez ebx
            fprintf(out, "    movl restore, %%ebx\n");
            
            // * e ca sa sar la adresa din variabila
            fprintf(out, "    jmp *jumpaddress\n");

            // printez label-ul folosit in caz ca nu se executa condjump-ul 
            fprintf(out, "label%d:\n", condjumps);
            // incrementez sufixul label-urilor
            condjumps++; 
        }

        else if (program[i].type == INSTR_JL) {
            fprintf(out, "    # Movfuscated JL %s\n", program[i].op1);

            // salvez si curat ebx 
            fprintf(out, "    movl %%ebx, restore\n");
            fprintf(out, "    movl $0, %%ebx\n");

            // mut adresa label-ului din op1
            fprintf(out, "    movl $%s, %%ebx\n", program[i].op1);
            fprintf(out, "    movl %%ebx, jumps+4\n");
            // mut adresa labelului "fictiv" la care sar daca cmp nu a produs flag-ul necesar sariturii
            fprintf(out, "    movl $label%d, %%ebx\n", condjumps);
            fprintf(out, "    movl %%ebx, jumps\n");
            // mut flags in ebx ca sa il folosesc pe post de offset cand caut in table-ul coresp instructiunii conditionale
            fprintf(out, "    movl flags, %%ebx\n");
            fprintf(out, "    movzbl jl_table(%%ebx), %%ebx\n");
            // mut adresa label-ului ales in jumpaddress
            fprintf(out, "    movl jumps(%%ebx), %%ebx\n");
            fprintf(out, "    movl %%ebx, jumpaddress\n");

            // restaurez ebx
            fprintf(out, "    movl restore, %%ebx\n");
            
            // * e ca sa sar la adresa din variabila
            fprintf(out, "    jmp *jumpaddress\n");

            // printez label-ul folosit in caz ca nu se executa condjump-ul 
            fprintf(out, "label%d:\n", condjumps);
            // incrementez sufixul label-urilor
            condjumps++; 
        }
        

        else if (program[i].type == INSTR_JLE) {
            fprintf(out, "    # Movfuscated JLE %s\n", program[i].op1);

            // salvez si curat ebx 
            fprintf(out, "    movl %%ebx, restore\n");
            fprintf(out, "    movl $0, %%ebx\n");

            // mut adresa label-ului din op1
            fprintf(out, "    movl $%s, %%ebx\n", program[i].op1);
            fprintf(out, "    movl %%ebx, jumps+4\n");
            // mut adresa labelului "fictiv" la care sar daca cmp nu a produs flag-ul necesar sariturii
            fprintf(out, "    movl $label%d, %%ebx\n", condjumps);
            fprintf(out, "    movl %%ebx, jumps\n");
            // mut flags in ebx ca sa il folosesc pe post de offset cand caut in table-ul coresp instructiunii conditionale
            fprintf(out, "    movl flags, %%ebx\n");
            fprintf(out, "    movzbl jle_table(%%ebx), %%ebx\n");
            // mut adresa label-ului ales in jumpaddress
            fprintf(out, "    movl jumps(%%ebx), %%ebx\n");
            fprintf(out, "    movl %%ebx, jumpaddress\n");

            // restaurez ebx
            fprintf(out, "    movl restore, %%ebx\n");
            
            // * e ca sa sar la adresa din variabila
            fprintf(out, "    jmp *jumpaddress\n");

            // printez label-ul folosit in caz ca nu se executa condjump-ul 
            fprintf(out, "label%d:\n", condjumps);
            // incrementez sufixul label-urilor
            condjumps++; 
        }
        

        else if (program[i].type == INSTR_JE) {
            fprintf(out, "    # Movfuscated JE %s\n", program[i].op1);

            // salvez si curat ebx 
            fprintf(out, "    movl %%ebx, restore\n");
            fprintf(out, "    movl $0, %%ebx\n");

            // mut adresa label-ului din op1
            fprintf(out, "    movl $%s, %%ebx\n", program[i].op1);
            fprintf(out, "    movl %%ebx, jumps+4\n");
            // mut adresa labelului "fictiv" la care sar daca cmp nu a produs flag-ul necesar sariturii
            fprintf(out, "    movl $label%d, %%ebx\n", condjumps);
            fprintf(out, "    movl %%ebx, jumps\n");
            // mut flags in ebx ca sa il folosesc pe post de offset cand caut in table-ul coresp instructiunii conditionale
            fprintf(out, "    movl flags, %%ebx\n");
            fprintf(out, "    movzbl je_table(%%ebx), %%ebx\n");
            // mut adresa label-ului ales in jumpaddress
            fprintf(out, "    movl jumps(%%ebx), %%ebx\n");
            fprintf(out, "    movl %%ebx, jumpaddress\n");

            // restaurez ebx
            fprintf(out, "    movl restore, %%ebx\n");
            
            // * e ca sa sar la adresa din variabila
            fprintf(out, "    jmp *jumpaddress\n");

            // printez label-ul folosit in caz ca nu se executa condjump-ul 
            fprintf(out, "label%d:\n", condjumps);
            // incrementez sufixul label-urilor
            condjumps++; 
        }


        else if (program[i].type == INSTR_JNE) {
            fprintf(out, "    # Movfuscated JNE %s\n", program[i].op1);

            // salvez si curat ebx 
            fprintf(out, "    movl %%ebx, restore\n");
            fprintf(out, "    movl $0, %%ebx\n");

            // mut adresa label-ului din op1
            fprintf(out, "    movl $%s, %%ebx\n", program[i].op1);
            fprintf(out, "    movl %%ebx, jumps+4\n");
            // mut adresa labelului "fictiv" la care sar daca cmp nu a produs flag-ul necesar sariturii
            fprintf(out, "    movl $label%d, %%ebx\n", condjumps);
            fprintf(out, "    movl %%ebx, jumps\n");
            // mut flags in ebx ca sa il folosesc pe post de offset cand caut in table-ul coresp instructiunii conditionale
            fprintf(out, "    movl flags, %%ebx\n");
            fprintf(out, "    movzbl jne_table(%%ebx), %%ebx\n");
            // mut adresa label-ului ales in jumpaddress
            fprintf(out, "    movl jumps(%%ebx), %%ebx\n");
            fprintf(out, "    movl %%ebx, jumpaddress\n");

            // restaurez ebx
            fprintf(out, "    movl restore, %%ebx\n");
            
            // * e ca sa sar la adresa din variabila
            fprintf(out, "    jmp *jumpaddress\n");

            // printez label-ul folosit in caz ca nu se executa condjump-ul 
            fprintf(out, "label%d:\n", condjumps);
            // incrementez sufixul label-urilor
            condjumps++; 
        }    
        // Loop il decrementeaza pe ecx, si daca e zero sare la label
        // practic dec %ecx, cmp $0, %ecx, je $label 


        else if (program[i].type == INSTR_LOOP) {
            fprintf(out, "    # Movfuscated LOOP %s\n", program[i].op1);

            // salvez ebx si il curat
            fprintf(out, "    movl %%ebx, restore\n");
            fprintf(out, "    movl $0, %%ebx\n");

            // decrementez ecx
            fprintf(out, "    movl %%ecx, %%ebx\n"); 
            fprintf(out, "    movzbl dec_table(%%ebx), %%ebx\n"); 
            fprintf(out, "    mov %%ebx, %%ecx\n");

            // logica de la cmp 
            // voi considera ca am cmp $0, %ecx 

            // in lookup table, i e dest, j e src, deci 
            // ecx va fi dest, 0 va fi src 
            // cmp face dest - src deci op2 - op1 = > op2 va fi i, op1 va fi j = > op1 va fi coloana, op2 va fi linia
            // deci op1 in bl, op2 in bh
            // salvez in val1, val2 operanzii
            fprintf(out, "    movl $0, %%ebx\n");
            fprintf(out, "    movl %%ebx, val1\n"); // src = ebx care e 0
            fprintf(out, "    movl %%ecx, val2\n"); // dest = ecx 
            

            // folosesc bx pentru accesarea adresei corespunzatoare, linia in bh (*256) si coloana in bl 
            fprintf(out, "    movb val1, %%bl\n");
            fprintf(out, "    movb val2, %%bh\n");
            fprintf(out, "    movzbl cmp_table(%%ebx), %%ebx\n");
            fprintf(out, "    movl %%ebx, flags\n");

            // in flags voi avea 0000 0000 0000 0000 0000 0000 0000 0ZSO (Z - zero, S - sign, O - overflow)
            // mut adresa label-ului care ii apartine lui loop in ebx
            fprintf(out, "    movl $%s, %%ebx\n", program[i].op1);
            fprintf(out, "    movl %%ebx, jumps+4\n");
            // mut adresa labelului "fictiv" la care sar daca cmp nu a produs flag-ul necesar sariturii
            fprintf(out, "    movl $label%d, %%ebx\n", condjumps);
            fprintf(out, "    movl %%ebx, jumps\n");
            // mut flags in ebx ca sa il folosesc pe post de offset cand caut in table-ul coresp instructiunii conditionale
            fprintf(out, "    movl flags, %%ebx\n");
            fprintf(out, "    movzbl jne_table(%%ebx), %%ebx\n");
            // mut adresa label-ului ales in jumpaddress
            fprintf(out, "    movl jumps(%%ebx), %%ebx\n");
            fprintf(out, "    movl %%ebx, jumpaddress\n");
            


            // restaurez ebx
            fprintf(out, "    movl restore, %%ebx\n");
            
            // * e ca sa sar la adresa din variabila
            fprintf(out, "    jmp *jumpaddress\n");

            // printez label-ul folosit in caz ca nu se executa condjump-ul 
            fprintf(out, "label%d:\n", condjumps);
            // incrementez sufixul label-urilor
            condjumps++; 
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

            if (strcmp(program[i].op1, "%%ebx") != 0){
                fprintf(out, "    movl %%ebx, restore\n");
            }

            fprintf(out, "    movl $0, %%ebx\n");       // Curățăm ebx (ca să folosim bl)
            fprintf(out, "    mov %s, %%ebx\n", program[i].op1); // Punem valoarea în ebx
            
            // Luam rezultatul din tabel: inc_table + index
            fprintf(out, "    movzbl inc_table(%%ebx), %%ebx\n"); 
            
            // Scriem rezultatul înapoi
            fprintf(out, "    mov %%ebx, %s\n", program[i].op1);

            if (strcmp(program[i].op1, "%%ebx") != 0) {
                fprintf(out, "    movl restore, %%ebx\n");
            }
        }

        else if (program[i].type == INSTR_DEC) {

            fprintf(out, "    # Movfuscated DEC\n");

            if (strcmp(program[i].op1, "%%ebx") != 0){
                fprintf(out, "    movl %%ebx, restore\n");
            }

            fprintf(out, "    movl $0, %%ebx\n");       // Curățăm ebx (ca să folosim bl)
            fprintf(out, "    mov %s, %%ebx\n", program[i].op1); // Punem valoarea în ebx
            
            // Luam rezultatul din tabel: dec_table + index
            fprintf(out, "    movzbl dec_table(%%ebx), %%ebx\n"); 
            
            // Scriem rezultatul înapoi
            fprintf(out, "    mov %%ebx, %s\n", program[i].op1);

            //restaurare ebx daca e nevoie
            if (strcmp(program[i].op1, "%%ebx") != 0) {
                fprintf(out, "    movl restore, %%ebx\n");
            }
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