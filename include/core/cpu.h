#ifndef PPLUSEMU_CPU_H
#define PPLUSEMU_CPU_H

#include <stdbool.h>
#include <stdint.h>

#include "mem.h"

typedef struct cpu {
	uint32_t ip;

	// Register Files
	uint32_t data[16];
	uint32_t addr[16];
	uint8_t cond;

	// Memory
	mem_t memory;
} cpu_t;

#pragma pack(push, 1)
typedef union instruction {
	uint32_t instr_word;
	struct {
		// First byte of the instruction
		uint8_t opcode;
		union {
			struct {
				// Lower half of the second byte
				unsigned src_r : 4;
				// Upper half of the second byte
				unsigned dst_r : 4;
				uint16_t _padding;
			} hrr;
		};
	};
} instruction_t;
#pragma pack(pop)

void cpu_reset(cpu_t *cpu, mem_t memory);
bool cpu_execute(cpu_t *cpu);

#endif // PPLUSEMU_CPU_H
