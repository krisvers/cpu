#include <cpu.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#define MEMORY_SIZE 8192

#define MEMORY_ERR -1
#define OPCODE_ERR -2

#define CPU_REGISTER_INDEX(n) \
	(*(reg_array[n]))

#define CPU_REGISTER_INDEX_SIGNED(n) \
	(*((int32_t *) (reg_array[n])))

struct cpuRegisters reg;
struct cpuInstruction last_instruction;

uint32_t * reg_array[9];
uint8_t memory[MEMORY_SIZE];
static int8_t err = 0;

static int8_t cpu_assign_reg_from_memory_byte(uint8_t reg, uint32_t vp) {
	if (vp > MEMORY_SIZE) {
		return MEMORY_ERR;
	}

	CPU_REGISTER_INDEX(reg) = (memory[vp]);

	return 0;
}

static int8_t cpu_assign_reg_from_memory_word(uint8_t reg, uint32_t vp) {
	if (vp + 1 > MEMORY_SIZE) {
		return MEMORY_ERR;
	}

	CPU_REGISTER_INDEX(reg) = (memory[vp]) | (memory[vp + 1] << 8);

	return 0;
}

static int8_t cpu_assign_reg_from_memory(uint8_t reg, uint32_t vp) {
	if (vp + 3 > MEMORY_SIZE) {
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

static void cpu_error(int8_t error) {
	err = error;
}

void cpu_init() {
	reg_array[0] = &reg.a;
	reg_array[1] = &reg.b;
	reg_array[2] = &reg.c;
	reg_array[3] = &reg.d;
	reg_array[4] = &reg.e;
	reg_array[5] = &reg.f;
	reg_array[6] = &reg.sp;
	reg_array[7] = &reg.bp;
	reg_array[8] = &reg.ip;
}

void cpu_print_status() {
	printf(
		"a: 0x%04x, b: 0x%04x, c: 0x%04x, d: 0x%04x, e: 0x%04x, f: 0x%04x\nsp: 0x%04x, bp: 0x%04x, ip: 0x%04x\n",
		reg.a,     reg.b,    reg.c,    reg.d,    reg.e,    reg.f,    reg.sp,    reg.bp,    reg.ip
	);
	printf(
		"instruction:\n\topcode: 0x%02x, mod: %d, info: 0x%01x, reg1: 0x%01x, reg2: 0x%01x, value: 0x%04x\n",
		last_instruction.instr, last_instruction.mod, last_instruction.info, last_instruction.reg1, last_instruction.reg2, last_instruction.value
	);
}

void cpu_load_memory(uint8_t * ptr, size_t len) {
	memcpy(memory, ptr, len);
}

struct cpuInstruction cpu_fetch_instruction() {
	if (reg.ip + 5 > MEMORY_SIZE) {
		cpu_error(MEMORY_ERR);

		return (struct cpuInstruction) {
			0, 0, 0, 0, 0, 0
		};
	}

	last_instruction.instr	= (memory[reg.ip] >> 3);
	last_instruction.mod	= (memory[reg.ip] >> 2) & 0b1;
	last_instruction.info	= (memory[reg.ip] & 0b11);
	last_instruction.reg1	= (memory[reg.ip + 1] >> 4);
	last_instruction.reg2	= (memory[reg.ip + 1] & 0b1111);
	last_instruction.value	= ((memory[reg.ip + 2]) | (memory[reg.ip + 3] << 8) | (memory[reg.ip + 4] << 16) | (memory[reg.ip + 5] << 24));

	return last_instruction;
}

int cpu_run_instruction(struct cpuInstruction instruction) {
	if (err) {
		return err;
	}

	switch (instruction.instr) {
		// nop
		case NOP:
			return 0;
		// load
		case LOD:
			// load reg, addr
			if (instruction.mod) {
				int8_t ret;
				switch (instruction.info) {
					case 0:
						ret = cpu_assign_reg_from_memory_byte(instruction.reg1, instruction.value);
						break;
					case 1:
						ret = cpu_assign_reg_from_memory_word(instruction.reg1, instruction.value);
						break;
					case 2:
						ret = cpu_assign_reg_from_memory(instruction.reg1, instruction.value);
						break;
					default:
						return OPCODE_ERR;
				}

				return ret;
			}

			// load reg, imm
			CPU_REGISTER_INDEX(instruction.reg1) = instruction.value;

			return 0;
		// store
		case STR:
			// store addr, reg
			if (instruction.mod) {
				int8_t ret;
				switch (instruction.info) {
					case 0:
						ret = cpu_assign_memory_from_reg_byte(instruction.reg1, instruction.value);
						break;
					case 1:
						ret = cpu_assign_memory_from_reg_word(instruction.reg1, instruction.value);
						break;
					case 2:
						ret = cpu_assign_memory_from_reg(instruction.reg1, instruction.value);
						break;
					default:
						return OPCODE_ERR;
				}

				return ret;
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
				int8_t ret;
				switch (instruction.info) {
					case 0:
						ret = cpu_assign_memory_from_reg_byte(instruction.reg1, reg.sp);
						break;
					case 1:
						ret = cpu_assign_memory_from_reg_word(instruction.reg1, reg.sp);
						break;
					case 2:
						ret = cpu_assign_memory_from_reg(instruction.reg1, reg.sp);
						break;
					default:
						return OPCODE_ERR;
				}

				reg.sp -= 4;
				return ret;
			}

			// pop reg
			reg.sp -= 4;
			if (cpu_assign_reg_from_memory(instruction.reg1, reg.sp) != 0) {
				return MEMORY_ERR;
			}

			return 0;
		case JMP:
			if (instruction.mod) {
				// jxx imm
				if (instruction.value + 5 > MEMORY_SIZE) {
					return MEMORY_ERR;
				}

				switch (instruction.info) {
					// jz imm
					case 0:
						if (!reg.c) {
							reg.ip = instruction.value;
						}
						break;
					// jg imm (signed)
					case 1:
						if (!(reg.c >> 31)) {
							reg.ip = instruction.value;
						}
						break;
					// jl imm (signed)
					case 2:
						if (reg.c >> 31) {
							reg.ip = instruction.value;
						}
						break;
					// jmp imm
					case 3:
						reg.ip = instruction.value;
						break;
				}

				return 0;
			}

			// jxx reg
			if (CPU_REGISTER_INDEX(instruction.reg1) + 5 > MEMORY_SIZE) {
				return MEMORY_ERR;
			}

			switch (instruction.info) {
				// jz reg
				case 0:
					if (!reg.c) {
						reg.ip = CPU_REGISTER_INDEX(instruction.reg1);
					}
					break;
				// jg reg (signed)
				case 1:
					if (!(reg.c >> 31)) {
						reg.ip = CPU_REGISTER_INDEX(instruction.reg1);
					}
					break;
				// jl reg (signed)
				case 2:
					if (reg.c >> 31) {
						reg.ip = CPU_REGISTER_INDEX(instruction.reg1);
					}
					break;
				// jmp reg
				case 3:
					reg.ip = CPU_REGISTER_INDEX(instruction.reg1);
					break;
			}

			return 0;
		default:
			return OPCODE_ERR;
	}
}
