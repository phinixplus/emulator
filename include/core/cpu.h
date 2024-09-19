#ifndef PPLUSEMU_CORE_CPU_H
#define PPLUSEMU_CORE_CPU_H

#include <stdbool.h>
#include <stdint.h>

#include "mem.h"
#include "io.h"

typedef struct cpu {
	uint64_t steps;

	// Architectural State
	uint32_t ip;
	uint32_t data[16];
	uint32_t addr[16];
	uint8_t cond;

	// Interfaces
	mem_t mem;
	io_t io;
} cpu_t;

extern const char datareg_conv[][3];
extern const char addrreg_conv[][3];

void cpu_reset(cpu_t *cpu, mem_t memory, io_t io);
bool cpu_execute(cpu_t *cpu);
void cpu_print_state(cpu_t *cpu);

#endif // PPLUSEMU_CORE_CPU_H
