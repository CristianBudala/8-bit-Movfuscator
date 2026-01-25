```
  ___        _     _ _                                   
 ( _ )      | |__ (_) |_                                 
 / _ \ _____| '_ \| | __|                                
| (_) |_____| |_) | | |_                                 
 \___/_     |_.__/|_|\__|                  _             
|  \/  | _____   __/ _|_   _ ___  ___ __ _| |_ ___  _ __ 
| |\/| |/ _ \ \ / / |_| | | / __|/ __/ _` | __/ _ \| '__|
| |  | | (_) \ V /|  _| |_| \__ \ (_| (_| | || (_) | |   
|_|  |_|\___/ \_/ |_|  \__,_|___/\___\__,_|\__\___/|_|

8-bit Movfuscator
Based on M/o/Vfuscator by Chris Domas - @xoreaxeaxeax

By Cristian Budala - @CristianBudala and Joshua Imosanu - @imo006

------------------------------------------------------------------

University of Bucharest
Department of Mathematics and Computer Science
Year 1, Sem 1: Computer Systems Arhitecture
```

## Overview
In 2013, PhD candidate Stephen Dolan from the University of Cambridge wrote an academic paper, stating that, in Assembly x86, the [mov instruction is Turing-complete](https://harrisonwl.github.io/assets/courses/malware/spring2017/papers/mov-is-turing-complete.pdf). After a series of rigorous logical proofs, he ended the paper (see attachment) by stating:
> "Removing all but the mov instruction from future iterations of
> the x86 architecture would have many advantages: the instruction format would be greatly simplified, the expensive decode unit
> would become much cheaper, and silicon currently used for complex functional units could be repurposed as even more cache.
> **As long as someone else implements the compiler.**"

This challenge has been taken by Cybersecurity Researcher Chris Domas [@xoreaxeaxeax](https://github.com/xoreaxeaxeax), in 2016, with his [Movfuscator](https://github.com/xoreaxeaxeax/movfuscator?tab=readme-ov-file) project: a compiler that transforms all known Assembly x86 instructions (arithmetic, comparisons, jumps, function calls etc.) into "mov" only instructions.
<br>
The current project is a minimalistic, educational compiler for numbers on 8-bits, inspired by Chris Domas's original Movfuscator.


🎓 This project was developed as part of the course "Computer Systems Arhitecture", under the supervision of [Prof. PhD. Cristian RUSU](https://www.researchgate.net/profile/Cristian-Rusu), at the Department for Mathematics and Computer Science, University of Bucharest.

## ❓ Why?
By reducing a program to almost exclusively `MOV` instructions, reverse engineering becomes virtually impossible. It wipes out the patterns that disassemblers and decompilers depend on, so the code stops “making sense”. The code becomes almost impossible to break - [still breakable](https://www.youtube.com/watch?v=d_R8i0dVBsQ), but the process becomes a lot lengthier and more exprensive.
Plus it's fun!

## ❗ Limitations
While the generated code runs on 32-bit x86 processors (using registers like `%eax`), the internal logic operates on an **8-bit architecture**.

* **The Problem:** To implement a lookup table for 32-bit operations, we would either need $2^{32} \times 2^{32}$ entries, requiring **18 Exabytes** of RAM, or very complex lookup tables, with flags implemented for every bit's potential overflow. (Chris Domas's solution)
* **The Solution:** We use 8-bit lookup tables (64KB) for arithmetic instructions.
* **The Logic:** We utilize the x86 register structure (`%bh` and `%bl`) to perform "hardware-free" index calculation. By placing operand A in `%bh` and operand B in `%bl`, the register `%bx` automatically becomes the linear index `(A * 256) + B`.

## ⚙️ How It Works

### 1. Arithmetic via Lookup
Every operation is translated into a sequence in memory and results are values fetched from the sequences:
* **INC:** Uses `inc_table` (Array). Input value is the index; output is the value at that index.
* **ADD/SUB:** Uses `add_table` and `sub_table` (Matrices in memory, values accessed with linear indexation).

### 2. Logic & Control Flow
We simulate boolean logic and comparisons without using standard `CMP` flag modifications directly.
* **Comparison:** `CMP A, B` is performed as a subtraction `B - A` via `sub_table`.
* **Zero Flag Simulation:** The result is passed through a `zero_table` (equivalent to a Logical NOT gate), which returns `1` if the difference is `0`, and `0` otherwise.
* **Jumps:** Conditional jumps (`JE`) operate by inspecting this simulated flag stored in memory (`flag_zf`) and linking it to the CPU's actual EFLAGS.

## ✅ Project Checklist (Movfuscated Instructions)

Current status of the pseudo-assembly instructions supported by the compiler:

| Instruction | Type | Status | Implementation Method |
| :--- | :--- | :--- | :--- |
| **MOV** | Data Transfer | ✅ **Done** | Direct translation to x86 `MOV`. |
| **INT** | Sys Call | ✅ **Done** | Linux `int $0x80` interface. |
| **INC** | Arithmetic | ✅ **Done** | `inc_table` (256 bytes). |
| **ADD** | Arithmetic | ✅ **Done** | `add_table` (64KB) via `%bx` indexing. |
| **SUB** | Arithmetic | ✅ **Done** | `sub_table` (64KB) via `%bx` indexing. |
| **CMP** | Logic | ✅ **Done** | Subtraction + `zero_table` pipelining. |
| **JE** | Control Flow | ✅ **Done** | Compares `flag_zf` with 1. |
| **JMP** | Control Flow | ✅ **Done** | Unconditional jump. |
| **LABEL** | Control Flow | ✅ **Done** | Assembly labels. |
| **XOR** | Logic | ❌ TBD |  |
| **JGE** | Control Flow | ❌ TBD |  |

## 🛠️ Usage Instructions

### Dependencies
* **OS:** Linux terminal.
* **Compiler:** GCC with 32-bit support (package `gcc-multilib`).

### 1. Cloning the Repo
Duh.
```bash
git clone https://github.com/CristianBudala/8-bit-Movfuscator
cd '8-bit-Movfuscator'
```
### 2. Build the Compiler
Compile the source code of the movfuscator itself:
```bash
gcc main.c parser.c emitter.c -o movf
```
### 3. Create Assembly file for Movfuscation
The code must be written in x86 AT&T syntax.<br>
**Note!** Use `#` for comments, not `;`.
```bash
touch test.s
nano test.s
```
### 4. Compile
```bash
./movf test.s
```
### 5. Assemble and Link
```bash
gcc -m32 output.s -o test -no-pie
```
### 6. Run
```bash
./test
echo $? # View the result
```
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
- [Wikipedia contributors, “One-instruction set computer,” Wikipedia, Dec. 18, 2025.](https://en.wikipedia.org/wiki/One-instruction_set_computer)
- "Theory of Computation"; Portland State University: Prof. Harry Porter; www.cs.pdx/~harry
- Google, Gemini 3. For debugging the code, fixing typos, double-checking my logic and understanding of certain concepts and for when I wanted to do something, but didn't have enough knowledge in the C Programming Language (maybe I should have picked Python).

## 📝 License
This project is MIT licensed.
