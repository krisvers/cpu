#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#include <cpu.h>

#define CODE_LENGTH 256

uint8_t code[CODE_LENGTH] = {
	
};

int main() {
	struct cpuInstruction instruction;
	int ret;

	cpu_load_memory(code, CODE_LENGTH);

	while (1) {
		instruction = cpu_fetch_instruction();
		
		ret = cpu_run_instruction(instruction);

		switch (ret) {
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
	}
}
