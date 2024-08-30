#include "cpu.h"

typedef union instruction {
	uint32_t instr_word;
	struct {
		uint8_t opcode;
		unsigned dst_r : 4;
		unsigned src_r : 4;
		uint16_t _padding;
	} hrr;
} instruction_t;

void cpu_reset(cpu_t *cpu, mem_t memory) {
	cpu->ip = 0;
	if(memory != NULL) cpu->memory = memory;
}

bool cpu_execute(cpu_t *cpu) {
	(void) cpu;
	return false;
}
