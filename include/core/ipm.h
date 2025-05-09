#ifndef PPLUSEMU_CORE_IPM_H
#define PPLUSEMU_CORE_IPM_H

#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>

#include <pthread.h>

#include "io.h"

typedef struct cpu cpu_t;
typedef struct ipm {
	atomic_bool is_init;
	bool is_privileged;
	uint32_t irq_bitmap;
} ipm_t;

void ipm_new(cpu_t *cpu);
void ipm_del(cpu_t *cpu);

void ipm_set_privilege(cpu_t *cpu, bool is_privileged);
bool ipm_check_privilege(cpu_t *cpu, bool irq_if_fail);
bool ipm_interrupt(cpu_t *cpu, uint16_t irq_id);
bool ipm_interrupted(cpu_t *cpu);

#endif // PPLUSEMU_CORE_IPM_H
