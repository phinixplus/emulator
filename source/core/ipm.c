#include "ipm.h"

#include <stdlib.h>

#include "cpu.h"

static void isrloc_callback(bool rw_select, uint32_t *rw_data, void *context) {
	cpu_t *context_real = (cpu_t *) context;
	if(rw_select) context_real->start_addr = *rw_data;
	else *rw_data = context_real->start_addr;
}

void ipm_new(ipm_t *ipm) {
	ipm->is_privileged = true;
}

bool ipm_attach_isrloc(io_t io, uint16_t port, cpu_t *context) {
	return io_attach_read(io, port, isrloc_callback, context)
		&& io_attach_write(io, port, isrloc_callback, context);
}

bool ipm_detach_isrloc(io_t io, uint16_t port) {
	return io_detach_read(io, port, NULL, NULL)
		&& io_detach_write(io, port, NULL, NULL);
}

void ipm_set_privilege(cpu_t *cpu, bool is_privileged) {
	if(cpu->ipm == NULL) return;
	cpu->ipm->is_privileged = is_privileged;
}

bool ipm_check_privilege(cpu_t *cpu, bool irq_if_fail) {
	if(cpu->ipm == NULL) return true;
	if(cpu->ipm->is_privileged) return true;
	if(irq_if_fail) cpu_interrupt(cpu);
	return false;
}
