#ifndef PPLUSEMU_CORE_IPM_H
#define PPLUSEMU_CORE_IPM_H

#include <stdbool.h>
#include <stdint.h>

#include "io.h"

typedef struct cpu cpu_t;
typedef struct ipm {
	bool is_privileged;
} ipm_t;

void ipm_new(ipm_t *ipm);

bool ipm_attach_isrloc(io_t io, uint16_t port, cpu_t *context);
bool ipm_detach_isrloc(io_t io, uint16_t port);

void ipm_set_privilege(cpu_t *cpu, bool is_privileged);
bool ipm_check_privilege(cpu_t *cpu, bool irq_if_fail);

#endif // PPLUSEMU_CORE_IPM_H
