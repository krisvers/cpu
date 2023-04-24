#ifndef CPU_H
#define CPU_H

#include <stdint.h>
#include <stddef.h>

#define NOP 0b00000
#define LOD 0b00001
#define STR 0b00010
#define ADD 0b00011
#define SUB 0b00100
#define MUL 0b00101
#define DIV 0b00110
#define OR  0b00111
#define AND 0b01000
#define XOR 0b01001
#define NOT 0b01010
#define SHR 0b01011
#define SHL 0b01100
#define PSH 0b01101
#define JMP 0b01110

struct cpuInstruction {
	uint8_t instr : 5;
	uint8_t mod : 1;
	uint8_t info : 2;
	uint8_t reg1 : 4;
	uint8_t reg2 : 4;
	uint32_t value : 32;
};

struct cpuRegisters {
	uint32_t a, b, c, d;
	uint32_t e, f;
	uint32_t sp, bp, ip;
};

struct cpuInstruction cpu_fetch_instruction();
void cpu_init();
void cpu_print_status();
int cpu_run_instruction(struct cpuInstruction instruction);
void cpu_load_memory(uint8_t * ptr, size_t len);

#endif
