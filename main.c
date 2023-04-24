#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#include <cpu.h>

#define CODE_LENGTH 480

uint8_t code[CODE_LENGTH] = {
	0b00001000, 0b00000000, 0xFF, 0x69, 0x00, 0x42
};

int main() {
	struct cpuInstruction instruction;
	int ret;

	cpu_load_memory(code, CODE_LENGTH);

	while (1) {
		instruction = cpu_fetch_instruction();
		
		ret = cpu_run_instruction(instruction);

		switch (ret) {
			case -2:
				fprintf(stderr, "CPU tried running invalid opcode\n");
				return -2;
			case -1:
				fprintf(stderr, "CPU tried accessing invalid memory\n");
				return -1;
			case 0:
				break;
			case 1:
				fprintf(stdout, "CPU powered off successfully\n");
				return 0;
			default:
				break;
		}

		printf("CPU:\n");
		cpu_print_status();

		while (fgetc(stdin) != '\n');
	}
}
