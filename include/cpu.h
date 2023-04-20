#ifndef CPU_H
#define CPU_H

#include <stdint.h>

struct cpuInstruction {
	uint8_t instr : 7;
	uint8_t mod : 1;
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
int cpu_run_instruction(struct cpuInstruction instruction);
void cpu_load_memory(uint8_t * ptr, size_t len);

#endif
