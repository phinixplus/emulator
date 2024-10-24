#ifndef PPLUSEMU_CORE_IPM_H
#define PPLUSEMU_CORE_IPM_H

#include <stdbool.h>
#include <stdint.h>

#include "io.h"

typedef struct cpu cpu_t;
typedef struct ipm {
	bool is_init;
	bool is_privileged;
} ipm_t;

void ipm_new(cpu_t *cpu);
void ipm_del(cpu_t *cpu);

void ipm_set_privilege(cpu_t *cpu, bool is_privileged);
bool ipm_check_privilege(cpu_t *cpu, bool irq_if_fail);

#endif // PPLUSEMU_CORE_IPM_H
