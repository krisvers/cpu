#include <cpu.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

#define MEMORY_SIZE 8192

#define CPU_REGISTER_INDEX(n) \
	(*(((uint32_t *) &reg) + n))

struct cpuRegisters reg;

uint8_t memory[MEMORY_SIZE];

static int8_t cpu_assign_reg_from_memory(uint8_t reg, uint32_t vp) {
	if (vp > MEMORY_SIZE) {
		return -1;
	}
	
	CPU_REGISTER_INDEX(reg) = *((uint32_t *) memory + vp);

	return 0;
}

struct cpuInstruction cpu_fetch_instruction() {
	return *((struct cpuInstruction *) &memory[reg.ip]);
}

int cpu_run_instruction(struct cpuInstruction instruction) {
	switch (instruction.instr) {
		// nop
		case 0b0000000:
			return 0;
		// load
		case 0b0000001:
			// load reg, addr
			if (instruction.mod) {
				if (cpu_assign_reg_from_memory(instruction.reg1, instruction.value) != 0) {
					return -1;
				}

				return 0;
			}

			// load reg, imm
			CPU_REGISTER_INDEX(instruction.reg1) = instruction.value;

			return 0;
		// store
		case 0b0000010:
			// store addr, reg
			if (instruction.mod) {
				
			}
			// store reg, reg
		default:
			return -2;
	}
}
