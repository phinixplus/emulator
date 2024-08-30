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

void cpu_reset(cpu_t *cpu, mem_t memory);
bool cpu_execute(cpu_t *cpu);

#endif // PPLUSEMU_CPU_H
