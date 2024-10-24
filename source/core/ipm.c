#include "ipm.h"

#include <assert.h>
#include <stdlib.h>

#include "cpu.h"

static void isrloc_callback(bool rw_select, uint32_t *rw_data, void *context) {
	cpu_t *context_real = (cpu_t *) context;
	if(rw_select) context_real->start_addr = *rw_data;
	else *rw_data = context_real->start_addr;
}

void ipm_new(cpu_t *cpu) {
	cpu->ipm.is_init = true;
	cpu->ipm.is_privileged = true;
	assert(io_try_attach(cpu->io, 0, IO_READWRITE, isrloc_callback, cpu));
}

void ipm_del(cpu_t *cpu) {
	cpu->ipm.is_init = false;
	assert(io_try_detach(cpu->io, 0, IO_READWRITE, NULL, NULL));
}

void ipm_set_privilege(cpu_t *cpu, bool is_privileged) {
	if(!cpu->ipm.is_init) return;
	cpu->ipm.is_privileged = is_privileged;
}

bool ipm_check_privilege(cpu_t *cpu, bool irq_if_fail) {
	if(!cpu->ipm.is_init) return true;
	if(cpu->ipm.is_privileged) return true;
	if(irq_if_fail) cpu_interrupt(cpu);
	return false;
}
