#ifndef PPLUSEMU_CORE_CPU_H
#define PPLUSEMU_CORE_CPU_H

#include <stdbool.h>
#include <stdint.h>

#include "io.h"
#include "ipm.h"
#include "mem.h"

typedef struct cpu {
	// Bookkeeping
	uint64_t step_count;
	uint32_t start_addr;
	pthread_mutex_t mutex;
	pthread_cond_t signal;

	// Architectural State
	uint32_t ip, jp;
	uint32_t data[16];
	uint32_t addr[16];
	uint8_t cond;

	// Interfaces
	mem_t mem;
	io_t io;
	ipm_t ipm;
} cpu_t;

void cpu_new(cpu_t *cpu, mem_t memory, io_t io);
void cpu_del(cpu_t *cpu);

void cpu_execute(cpu_t *cpu);

#endif // PPLUSEMU_CORE_CPU_H
