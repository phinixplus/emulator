#include "ipm.h"

#include <assert.h>
#include <stdlib.h>

#include "cpu.h"
#include "io/portdefs.h"

static void isrloc_callback(bool rw_select, uint32_t *rw_data, void *context) {
	cpu_t *context_real = (cpu_t *) context;
	if(rw_select) context_real->start_addr = *rw_data;
	else *rw_data = context_real->start_addr;
}

static void irqsrc_callback(bool rw_select, uint32_t *rw_data, void *context) {
	cpu_t *context_real = (cpu_t *) context;
	pthread_mutex_lock(&context_real->ipm.irq_mutex);
	if(rw_select) context_real->ipm.irq_bitmap &= ~*rw_data;
	else *rw_data = context_real->ipm.irq_bitmap;
	pthread_mutex_unlock(&context_real->ipm.irq_mutex);
}

void ipm_new(cpu_t *cpu) {
	if(cpu->ipm.is_init) return;
	cpu->ipm.is_init = true;
	cpu->ipm.is_privileged = true;
	cpu->ipm.irq_bitmap = 0;
	pthread_mutex_init(&cpu->ipm.irq_mutex, NULL);
	assert(io_try_attach(cpu->io, IO_ISRLOC, IO_READWRITE, isrloc_callback, cpu));
	assert(io_try_attach(cpu->io, IO_IRQSRC, IO_READWRITE, irqsrc_callback, cpu));
}

void ipm_del(cpu_t *cpu) {
	if(!cpu->ipm.is_init) return;
	cpu->ipm.is_init = false;
	pthread_mutex_destroy(&cpu->ipm.irq_mutex);
	assert(io_try_detach(cpu->io, IO_ISRLOC, IO_READWRITE, NULL, NULL));
	assert(io_try_detach(cpu->io, IO_IRQSRC, IO_READWRITE, NULL, NULL));
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

bool ipm_interrupt(cpu_t *cpu, uint16_t irq_id) {
	irq_id &= 0x1F;
	pthread_mutex_lock(&cpu->ipm.irq_mutex);
	bool succ = (cpu->ipm.irq_bitmap & 1 << irq_id) != 0;
	cpu->ipm.irq_bitmap |= 1 << irq_id;
	pthread_mutex_unlock(&cpu->ipm.irq_mutex);
	return succ;
}
