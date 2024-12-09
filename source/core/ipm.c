#include "ipm.h"

#include <assert.h>
#include <stdlib.h>

#include "cpu.h"
#include "main.h"
#include "io/portdefs.h"

static void isrloc_callback(bool rw_select, uint32_t *rw_data, void *context) {
	cpu_t *context_real = (cpu_t *) context;
	if(rw_select) context_real->start_addr = *rw_data;
	else *rw_data = context_real->start_addr;
}

static void irqsrc_callback(bool rw_select, uint32_t *rw_data, void *context) {
	cpu_t *context_real = (cpu_t *) context;
	if(rw_select) context_real->ipm.irq_bitmap &= ~*rw_data;
	else *rw_data = context_real->ipm.irq_bitmap;
}

static void envcmd_callback(bool rw_select, uint32_t *rw_data, void *context) {
	(void) context;
	assert(rw_select);
	if(*rw_data == 0)
		stop_running();
}

void ipm_new(cpu_t *cpu) {
	if(cpu->ipm.is_init) return;
	cpu->ipm.is_init = true;
	cpu->ipm.is_privileged = true;
	cpu->ipm.irq_bitmap = 0;
	assert(io_try_attach(cpu->io, IO_ISRLOC, isrloc_callback, cpu));
	assert(io_try_attach(cpu->io, IO_IRQSRC, irqsrc_callback, cpu));
	assert(io_try_attach(cpu->io, IO_ENVCMD, envcmd_callback, NULL));
}

void ipm_del(cpu_t *cpu) {
	if(!cpu->ipm.is_init) return;
	cpu->ipm.is_init = false;
	assert(io_try_detach(cpu->io, IO_ISRLOC, NULL, NULL));
	assert(io_try_detach(cpu->io, IO_IRQSRC, NULL, NULL));
	assert(io_try_detach(cpu->io, IO_ENVCMD, NULL, NULL));
}

void ipm_set_privilege(cpu_t *cpu, bool is_privileged) {
	if(!cpu->ipm.is_init) return;
	cpu->ipm.is_privileged = is_privileged;
}

bool ipm_check_privilege(cpu_t *cpu, bool irq_if_fail) {
	if(!cpu->ipm.is_init) return true;
	if(cpu->ipm.is_privileged) return true;
	if(irq_if_fail) ipm_interrupt(cpu, 0);
	return false;
}

bool ipm_interrupt(cpu_t *cpu, uint16_t irq_id) {
	irq_id &= 0x1F;
	pthread_mutex_lock(&cpu->mutex);
	bool succ = (cpu->ipm.irq_bitmap & 1 << irq_id) != 0;
	cpu->ipm.irq_bitmap |= 1 << irq_id;
	pthread_mutex_unlock(&cpu->mutex);
	return succ;
}

bool ipm_interrupted(cpu_t *cpu) {
	if(!cpu->ipm.is_init) return false;
	return cpu->ipm.irq_bitmap != 0;
}
