#include <cpu.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#define MEMORY_SIZE 8192

#define MEMORY_ERR -1
#define OPCODE_ERR -2

#define NOP 0b0000000
#define LOD 0b0000001
#define STR 0b0000010
#define ADD 0b0000011
#define SUB 0b0000100
#define MUL 0b0000101
#define DIV 0b0000110
#define OR  0b0000111
#define AND 0b0001000
#define XOR 0b0001001
#define NOT 0b0001010
#define SHR 0b0001011
#define SHL 0b0001100
#define PSH 0b0001101
#define JMP 0b0001110

#define CPU_REGISTER_INDEX(n) \
	(*(((uint32_t *) &reg) + n))

#define CPU_REGISTER_INDEX_SIGNED(n) \
	(*(((int32_t *) &reg) + n))

struct cpuRegisters reg;

uint8_t memory[MEMORY_SIZE];

static int8_t cpu_assign_reg_from_memory(uint8_t reg, uint32_t vp) {
	if (vp > MEMORY_SIZE) {
		return MEMORY_ERR;
	}

	CPU_REGISTER_INDEX(reg) = (memory[vp]) | (memory[vp + 1] << 8) | (memory[vp + 2] << 16) | (memory[vp + 3] << 24);

	return 0;
}

static int8_t cpu_assign_memory_from_reg_byte(uint8_t reg, uint32_t vp) {
	if (vp > MEMORY_SIZE) {
		return MEMORY_ERR;
	}

	uint8_t r = CPU_REGISTER_INDEX(reg);

	memory[vp] = r;

	return 0;
}

static int8_t cpu_assign_memory_from_reg_word(uint8_t reg, uint32_t vp) {
	if (vp + 1 > MEMORY_SIZE) {
		return MEMORY_ERR;
	}

	uint16_t r = CPU_REGISTER_INDEX(reg);

	memory[vp] = r & 0xFF;
	memory[vp + 1] = (r >> 8) & 0xFF;

	return 0;
}

static int8_t cpu_assign_memory_from_reg(uint8_t reg, uint32_t vp) {
	if (vp + 3 > MEMORY_SIZE) {
		return MEMORY_ERR;
	}

	uint32_t r = CPU_REGISTER_INDEX(reg);

	memory[vp] = (r & 0xFF);
	memory[vp + 1] = (r >> 8) & 0xFF;
	memory[vp + 2] = (r >> 16) & 0xFF;
	memory[vp + 3] = (r >> 24) & 0xFF;

	return 0;
}

void cpu_load_memory(uint8_t * ptr, size_t len) {
	memcpy(memory, ptr, len);
}

struct cpuInstruction cpu_fetch_instruction() {
	return *((struct cpuInstruction *) &memory[reg.ip]);
}

int cpu_run_instruction(struct cpuInstruction instruction) {
	switch (instruction.instr) {
		// nop
		case NOP:
			return 0;
		// load
		case LOD:
			// load reg, addr
			if (instruction.mod) {
				if (cpu_assign_reg_from_memory(instruction.reg1, instruction.value) != 0) {
					return MEMORY_ERR;
				}

				return 0;
			}

			// load reg, imm
			CPU_REGISTER_INDEX(instruction.reg1) = instruction.value;

			return 0;
		// store
		case STR:
			// store addr, reg
			if (instruction.mod) {
				if (cpu_assign_memory_from_reg(instruction.reg1, instruction.value) != 0) {
					return MEMORY_ERR;
				}

				return 0;
			}

			// store reg, reg
			CPU_REGISTER_INDEX(instruction.reg1) = CPU_REGISTER_INDEX(instruction.reg2);

			return 0;
		// add
		case ADD:
			// add reg, imm
			if (instruction.mod) {
				CPU_REGISTER_INDEX(instruction.reg1) += instruction.value;

				return 0;
			}

			// add reg, reg
			CPU_REGISTER_INDEX(instruction.reg1) += CPU_REGISTER_INDEX(instruction.reg2);

			return 0;
		// sub
		case SUB:
			// sub reg, imm
			if (instruction.mod) {
				CPU_REGISTER_INDEX(instruction.reg1) -= instruction.value;

				return 0;
			}

			// sub reg, reg
			CPU_REGISTER_INDEX(instruction.reg1) -= CPU_REGISTER_INDEX(instruction.reg2);

			return 0;
		// mul
		case MUL:
			// muls reg, reg
			if (instruction.mod) {
				reg.e = CPU_REGISTER_INDEX_SIGNED(instruction.reg1) * CPU_REGISTER_INDEX_SIGNED(instruction.reg2);

				return 0;
			}

			// mulu reg, reg
			reg.e = CPU_REGISTER_INDEX(instruction.reg1) * CPU_REGISTER_INDEX(instruction.reg2);

			return 0;
		// div
		case DIV:
			// divs reg, reg
			if (instruction.mod) {
				reg.e = CPU_REGISTER_INDEX_SIGNED(instruction.reg1) % CPU_REGISTER_INDEX_SIGNED(instruction.reg2);
				reg.f = CPU_REGISTER_INDEX_SIGNED(instruction.reg1) / CPU_REGISTER_INDEX_SIGNED(instruction.reg2);

				return 0;
			}

			// divu reg, reg
			reg.e = CPU_REGISTER_INDEX(instruction.reg1) % CPU_REGISTER_INDEX_SIGNED(instruction.reg2);
			reg.f = CPU_REGISTER_INDEX(instruction.reg1) / CPU_REGISTER_INDEX_SIGNED(instruction.reg2);

			return 0;
		// or
		case OR:
			// or reg, imm
			if (instruction.mod) {
				reg.c = CPU_REGISTER_INDEX(instruction.reg1) | instruction.value;

				return 0;
			}

			// or reg, reg
			reg.c = CPU_REGISTER_INDEX(instruction.reg1) | CPU_REGISTER_INDEX(instruction.reg2);

			return 0;
		// and
		case AND:
			// and reg, imm
			if  (instruction.mod) {
				reg.c = CPU_REGISTER_INDEX(instruction.reg1) & instruction.value;

				return 0;
			}

			// and reg, reg
			reg.c = CPU_REGISTER_INDEX(instruction.reg1) & CPU_REGISTER_INDEX(instruction.reg2);

			return 0;
		// xor
		case XOR:
			// xor reg, imm
			if (instruction.mod) {
				reg.c = CPU_REGISTER_INDEX(instruction.reg1) ^ instruction.value;

				return 0;
			}

			// xor reg, reg
			reg.c = CPU_REGISTER_INDEX(instruction.reg1) ^ CPU_REGISTER_INDEX(instruction.reg2);

			return 0;
		// not
		case NOT:
			// not imm
			if (instruction.mod) {
				reg.c = ~instruction.value;

				return 0;
			}

			// not reg
			reg.c = ~CPU_REGISTER_INDEX(instruction.reg1);

			return 0;
		// shr
		case SHR:
			// shr reg, imm
			if (instruction.mod) {
				reg.c = CPU_REGISTER_INDEX(instruction.reg1) >> instruction.value;

				return 0;
			}

			// shr reg, reg
			reg.c = CPU_REGISTER_INDEX(instruction.reg1) >> CPU_REGISTER_INDEX(instruction.reg2);

			return 0;
		// shl
		case SHL:
			// shl reg, imm
			if (instruction.mod) {
				reg.c = CPU_REGISTER_INDEX(instruction.reg1) << instruction.value;

				return 0;
			}

			// shl reg, reg
			reg.c = CPU_REGISTER_INDEX(instruction.reg1) << CPU_REGISTER_INDEX(instruction.reg2);

			return 0;
		// push / pop
		case PSH:
			// push reg
			if (instruction.mod) {
				if (cpu_assign_memory_from_reg(CPU_REGISTER_INDEX(instruction.reg1), reg.sp) != 0) {
					return MEMORY_ERR
				}

				reg.sp -= 4;
			}
		default:
			return OPCODE_ERR;
	}
}
