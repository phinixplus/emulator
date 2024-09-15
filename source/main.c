#include <assert.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#include "args.h"
#include "util.h"
#include "core/cpu.h"
#include "core/mem.h"
#include "core/io.h"
#include "io/dbgcon.h"
#include "io/telnet.h"

static options_t options;
static struct {
	pthread_t freq;
	mem_t mem;
	io_t io;
} cleanup_registry;

static void cleanup(void) {
	dbgcon_close(cleanup_registry.io);
	telnet_close(cleanup_registry.io);

	io_del(cleanup_registry.io);
	mem_del(cleanup_registry.mem);
	if(!options.verbose) {
		pthread_cancel(cleanup_registry.freq);
		pthread_join(cleanup_registry.freq, NULL);
	}
}

int main(int argc, char **argv) {
	options = args_parse(argc, argv);
	atexit(cleanup);

	// Exit gracefully on SIGINT
	struct sigaction sig_int;
	sig_int.sa_handler = exit_as_sighandler;
	sigemptyset(&sig_int.sa_mask);
	sigaddset(&sig_int.sa_mask, SIGTERM);
	sig_int.sa_flags = 0;
	sigaction(SIGINT, &sig_int, NULL);

	// Setup IO
	io_t io = io_new();
	cleanup_registry.io = io;
	assert(dbgcon_setup(io));
	assert(telnet_setup(io, 2323));

	// Setup memory and CPU
	mem_t mem = mem_new(options.file);
	cleanup_registry.mem = mem;
	cpu_t cpu; cpu_reset(&cpu, mem, io);

	// Prevent threads from trying to handle signals
	// Signal mask is inherited by spawned threads
	sigset_t signal_mask; sigemptyset(&signal_mask);
	sigaddset(&signal_mask, SIGINT);
	pthread_sigmask(SIG_BLOCK, &signal_mask, NULL);
	{ // <- Spawn threads inside this block
		if(!options.verbose) pthread_create(
			&cleanup_registry.freq,NULL, freq_thread, &cpu.steps);
	}
	// Revert the mask so that main thread gets to handle the signals
	pthread_sigmask(SIG_UNBLOCK, &signal_mask, NULL);

	// Do cycles and print sate if verbose
	do { if(options.verbose) print_cpu_state(&cpu); }
	while(cpu_execute(&cpu));
	fprintf(stderr, "CPU halted after %lu step(s).\n", cpu.steps);

	exit(EXIT_SUCCESS);
}
