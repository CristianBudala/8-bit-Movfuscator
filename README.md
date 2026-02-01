  \_\_\_        \_     \_ \_                                   

 ( \_ )      | |\_\_ (\_) |\_                                 

 / \_ \\ \_\_\_\_\_| '\_ \\| | \_\_|                                

| (\_) |\_\_\_\_\_| |\_) | | |\_                                 

 \\\_\_\_/\_     |\_.\_\_/|\_|\\\_\_|                  \_             

|  \\/  | \_\_\_\_\_   \_\_/ \_|\_   \_ \_\_\_  \_\_\_ \_\_ \_| |\_ \_\_\_  \_ \_\_ 

| |\\/| |/ \_ \\ \\ / / |\_| | | / \_\_|/ \_\_/ \_\` | \_\_/ \_ \\| '\_\_|

| |  | | (\_) \\ V /|  \_| |\_| \\\_\_ \\ (\_| (\_| | || (\_) | |   

|\_|  |\_|\\\_\_\_/ \\\_/ |\_|  \\\_\_,\_|\_\_\_/\\\_\_\_\\\_\_,\_|\\\_\_\\\_\_\_/|\_|

8-bit Movfuscator

Based on M/o/Vfuscator by Chris Domas \- @xoreaxeaxeax

By Cristian Budala \- @CristianBudala and Joshua Imosanu \- @imo006

\------------------------------------------------------------------

University of Bucharest

Department of Mathematics and Computer Science

Year 1, Sem 1: Computer Systems Arhitecture

## Overview

In 2013, PhD candidate Stephen Dolan from the University of Cambridge wrote an academic paper, stating that, in Assembly x86, the [mov instruction is Turing-complete](https://harrisonwl.github.io/assets/courses/malware/spring2017/papers/mov-is-turing-complete.pdf). After a series of rigorous logical proofs, he ended the paper (see attachment) by stating:

"Removing all but the mov instruction from future iterations of the x86 architecture would have many advantages: the instruction format would be greatly simplified, the expensive decode unit would become much cheaper, and silicon currently used for complex functional units could be repurposed as even more cache. **As long as someone else implements the compiler.**"

This challenge has been taken by Cybersecurity Researcher Chris Domas [@xoreaxeaxeax](https://github.com/xoreaxeaxeax), in 2016, with his [Movfuscator](https://github.com/xoreaxeaxeax/movfuscator?tab=readme-ov-file) project: a compiler that transforms all known Assembly x86 instructions (arithmetic, comparisons, jumps, function calls etc.) into "mov" only instructions. 

The current project is a more minimal, educational compiler for numbers on 8-bits, inspired by Chris Domas's original Movfuscator.

🎓 This project was developed as part of the course "Computer Systems Arhitecture", under the supervision of [Prof. PhD. Cristian RUSU](https://www.researchgate.net/profile/Cristian-Rusu), at the Department for Mathematics and Computer Science, University of Bucharest.

## ❓ Why?

By reducing a program to almost exclusively `MOV` instructions, reverse engineering becomes virtually impossible. It wipes out the patterns that disassemblers and decompilers depend on, so the code stops “making sense”. The code becomes almost impossible to break \- [still breakable](https://www.youtube.com/watch?v=d_R8i0dVBsQ), but the process becomes a lot lengthier and more exprensive. Plus it's fun\!

## ❗ Limitations

While the generated code runs on 32-bit x86 processors (using registers like `%eax`), the internal logic operates on an **8-bit architecture**.

* **The Problem:** To implement a lookup table for 32-bit operations, we would either need $2^{32} \\times 2^{32}$ entries, requiring **18 Exabytes** of RAM, or very complex lookup tables, with flags implemented for every bit's potential overflow. (Chris Domas's solution)  
* **The Solution:** We use 8-bit lookup tables (64KB) for arithmetic instructions along with some clever tricks.  
* **The Logic:** We utilize the x86 register structure (`%bh` and `%bl`) to perform "hardware-free" index calculation. By placing operand A in `%bh` and operand B in `%bl`, the register `%bx` automatically becomes the linear index `(A * 256) + B`.  
* **Certain instructions are not movfuscated**, such as push/pop or jmp, either due to the mov instruction’s limitations (we can’t use the eip register as a destination for mov, we can’t send system interrupts using only mov), or limitations in regards to how we implement the movfuscated arithmetic instructions (for instance, we would need a significantly more complex lookup table system if we were to handle push/pop, since they work with values higher than our 8-bits can cover).  
* **Certain instructions’ functionality may be limited** (lea only works if the src is a variable label). Flags are not updated, and instructions that modify them will instead modify a variable named flags inside .data.   
* **Not all x86 instructions are covered.**

## ⚙️ How It Works

1. ### Generating the .data section

For movfuscating each instruction, we will start by generating a lookup table for it in the .data section. This applies for every arithmetic (add, sub, inc, dec, mul, div), logic (xor, or, and, not), shift, conditional jump (jg, jge, jl, jle, je, jne) and cmp. Certain instructions (looking at you shifts, as well as loop) use more than one lookup table.   
Some instructions also make use of some additional variables we declare in .data, such as a jumps array that contains addresses for the movfuscated conditional jumps. A .long restore variable is also used to store & restore the value inside ebx whenever we movfuscate an instruction, since bx will be used as an index.   
Each instruction’s lookup table(s) generation process, as well as the thinking behind it will be more thoroughly explained inside the comments.  
The **source code** features ample details for the behind the scenes processes and we **strongly encourage you to at least skim through it** for a better understanding.

2. ### Using clever tricks to fetch data from the previously generated lookup tables

Most movfuscated instructions follow the pattern of:

- Save ebx in a variable created by us previously in .data   
- Clear ebx (movl $0, %ebx)  
- Place the index of the row inside bh, where row is one of the operands  
- Place the index of the column inside bl, where column is, again, one of the operands  
- Use bx to access the value we need from the lookup table (256 \* bh \+ bl will be the index)  
- Move whatever it is we need from the lookup table to ebx, and then to the designated destination  
- Restore the value ebx previously held (only if it was not the destination)

3. ###  Handling Logic & Control Flow

The idea of a conditional jump no longer exists in our world. We always jump, it’s just a matter of where we jump to. 

* CMP is movfuscated, and now updates a variable called flags previously declared inside .data. We structure flags so that each flag we need (zero, sign, overflow) will correspond to one of the first 3 bits (1 if the flag is on, 0 if it is not).  
* We store 2 addresses inside of an array, one for the address to which we will jump if the conditions are met, and another address which will belong to a label we will create right after the standard jmp instruction we insert at the end of the movfuscated conditional jump code block.   
* Each conditional jump instruction (including loop) will have its own lookup table that contains the offset used for updating the variable jumpaddress which will hold the address the jump should be executed to.  
* We fetch the respective offset from the conditional jump instruction’s lookup table using the flag variable as the index.   
* We use this offset to move the desired jump address to the jumpaddress variable we’ve previously mentioned, and we execute a standard jmp \*jumpaddress.  
* The loop instruction is turned into dec %ecx, cmp $0, %ecx and jne label, all of which are movfuscated going by the above logic. 




## ✅ Project Checklist (Movfuscated Instructions)

Current status of the pseudo-assembly instructions supported by the compiler:

| Instruction | Type | Status | Implementation Method |
| :---- | :---- | :---- | :---- |
| **MOV** | Data Transfer | ✅ **Done** | Direct translation to x86 `MOV`. |
| **INT** | Sys Call | ✅ **Done** | Linux `int $0x80` interface. |
| **INC** | Arithmetic | ✅ **Done** | `inc_table` (256 bytes). |
| **ADD** | Arithmetic | ✅ **Done** | `add_table` (64KB) via `%bx` indexing. |
| **SUB**  | Arithmetic | ✅ **Done** | `sub_table` (64KB) via `%bx` indexing. |
| **MUL** | Arithmetic | ✅ **Done** | `mul_table (128kb) via %bx indexing.` |
| **DIV** | Arithmetic | ✅ **Done** | `div_table (128kb) via %bx indexing.` |
| **CMP** | Logic | ✅ **Done** | cmp\_table, updates the flag variable |
| **JE/JNE/JG/JGE/JL/JLE** | Control Flow | ✅ **Done** | Uses the flag variable to determine what to use as the dest for the unconditional jmp instruction. |
| **LOOP** | Control Flow | ✅ **Done** | Treated as: dec %ecx, cmp $0, %ecx, jne label and then movfuscated. |
| **JMP** | Control Flow | ✅ **Done** | Unconditional jump. |
| **LABEL** | Control Flow | ✅ **Done** | Assembly labels. |
| **AND/OR/NOT/XOR** | Logic  | ✅ **Done** | inst\_table via %bx indexing |
| **SHL/SHR/SAR/SAL** | Shift | ✅ **Done** | Shift\_table for the src, shift\_inst\_table via %bx indexing |
| **LEA** | Logic | ✅ **Done** | Only works if the src is a variable\! |
| **PUSH/POP** | Logic | ✅ **Done** | Not movfuscated. |
| **CALL** | Control Flow | ✅ **Done** | Not movfuscated. |

## Details behind each implementation and design choice we’ve made is inside the source code. We’ve made sure to leave ample explanations inside the comments. We suggest you check them out. 

## 🛠️ Usage Instructions

### Dependencies

* **OS:** Linux terminal.  
* **Compiler:** GCC with 32-bit support (package `gcc-multilib`).

### 1\. Clone the Repo

Duh.

git clone https://github.com/CristianBudala/8-bit-Movfuscator

cd '8-bit-Movfuscator'

### 2\. Build the Compiler

Compile the source code of the movfuscator itself:

gcc main.c parser.c emitter.c \-o movf

### 3\. Create Assembly file for Movfuscation

The code must be written in x86 AT\&T syntax.

\*\*Note\!\*\* Use \`\#\` for comments, not \`;\`.

touch test.s

nano test.s

### 4\. Compile

./movf test.s

### 5\. Assemble and Link

gcc \-m32 output.s \-o test \-no-pie

### 6\. Run

./test

echo $? \# View the result

## 👥 Authors

- Cristian Budala (@CristianBudala)  
- Joshua Imosanu (@Imo006)

## References

- [Dolan, S. (2013). mov is Turing-complete.](https://harrisonwl.github.io/assets/courses/malware/spring2017/papers/mov-is-turing-complete.pdf)  
- [Domas, C. (2015). movfuscator](https://github.com/xoreaxeaxeax/movfuscator?tab=readme-ov-file)  
- [Domas, C. (2015). The MoVfuscator: Turning "mov" into a soul crushing RE nightmare Christopher Domas](https://www.youtube.com/watch?v=R7EEoWg6Ekk)  
- [Computerphile (2015). Turing Machines Collection](https://www.youtube.com/playlist?list=PLzH6n4zXuckrEzV0CB1xXbSdsP_a7VUoK)  
- [Kirsch, J., & Jonischkeit, C. (2016). Movfuscator Be Gone](https://www.youtube.com/watch?v=d_R8i0dVBsQ)  
- [Domas, C. (2015). Breaking the x86 Instruction Set](https://www.youtube.com/watch?v=KrksBdWcZgQ&list=PLyp2y3L9yp30eiGQPUghXEBEwHDrwMCw5&index=2)  
- [Easy Theory (2021). Turing Machines (TMs)](https://www.youtube.com/playlist?list=PLylTVsqZiRXNkiHYuOMLrQAxmmRt5qDct)  
- [Intel Corporation. Intel® 64 and IA-32 Architectures Software Developer’s Manuals.](https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html)  
- [Wikipedia contributors, “One-instruction set computer,” Wikipedia, Dec. 18, 2025\.](https://en.wikipedia.org/wiki/One-instruction_set_computer)  
- "Theory of Computation"; Portland State University: Prof. Harry Porter; [www.cs.pdx/\~harry](http://www.cs.pdx/~harry)  
- Google Gemini 3\. For debugging the code, fixing typos, double-checking logic, understanding of certain concepts and for gaps in our knowledge regarding the C programming language and its syntax (we probably should’ve gone with Python). 

## 📝 License

This project is MIT licensed.  
