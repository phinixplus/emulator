#include <assert.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#include "util.h"
#include "core/cpu.h"
#include "core/mem.h"
#include "core/io.h"

#include "io/dbgcon.h"
#include "io/telnet.h"

int main(int argc, char **argv) {
	if(argc != 2) {
		fprintf(stderr, "Usage: %s <memfile>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	signal(SIGPIPE, SIG_IGN);

	// Setup IO
	io_t io = io_new();
	assert(dbgcon_init(io));
	assert(telnet_init(io, 2323));

	// Setup Memory and CPU
	mem_t mem = mem_new(argv[1]);
	cpu_t cpu; cpu_reset(&cpu, mem, io);

	uint64_t count = 0;
	for(; print_cpu_state(count, &cpu), cpu_execute(&cpu); count++);
	fprintf(stderr, "CPU halted after %lu step(s).\n", count);

	telnet_cleanup();
	exit(EXIT_SUCCESS);
}
