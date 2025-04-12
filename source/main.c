#include "main.h"

#include <assert.h>
#include <signal.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pthread.h>
#include <unistd.h>

#include "args.h"
#include "core/cpu.h"
#include "core/mem.h"
#include "core/io.h"
#include "io/dbgcon.h"
#include "io/tty.h"

static options_t options;
static struct {
	pthread_t freq;
	cpu_t *cpu;
	mem_t mem;
	io_t io;
} cleanup_registry;

static atomic_bool running = true;
bool is_running(void) { return running; }
void stop_running(void) { running = false; }

void exit_as_sighandler(int sigid) {
	fprintf(stderr, "Caught signal: %s\n", strsignal(sigid));
	exit(EXIT_FAILURE);
}

static void *freq_counter_thread(void *cycle_count) {
	uint64_t *count = (uint64_t *) cycle_count;
	uint64_t old_count = 0;
	while(is_running()) {
		sleep(1);
		uint64_t tmp = *count;
		float freq_mhz = (tmp - old_count) / 1e6;
		fprintf(stderr, "%.1f MHz\n", freq_mhz);
		old_count = tmp;
	}
	pthread_exit(NULL);
}

static void cleanup(void) {
	stop_running(); // Cooperative shutdown signal (making sure)
	if(options.show_freq) pthread_join(cleanup_registry.freq, NULL);

	assert(dbgcon_close(cleanup_registry.io));
	assert(tty_close(cleanup_registry.io));

	cpu_del(cleanup_registry.cpu);
	mem_del(cleanup_registry.mem);
	io_del(cleanup_registry.io);
}

int main(int argc, char **argv) {
	options = args_parse(argc, argv);
	atexit(cleanup);

	// Exit gracefully on SIGINT
	struct sigaction sig_int;
	sig_int.sa_handler = exit_as_sighandler;
	sigemptyset(&sig_int.sa_mask);
	sigaddset(&sig_int.sa_mask, SIGINT);
	sig_int.sa_flags = 0;
	sigaction(SIGINT, &sig_int, NULL);

	// Setup and register for cleanup
	cpu_t cpu;
	cleanup_registry.cpu = &cpu;
	mem_t mem = mem_new(options.file);
	cleanup_registry.mem = mem;
	io_t io = io_new();
	cleanup_registry.io = io;

	// Prevent threads from trying to handle signals
	// Signal mask is inherited by spawned threads
	sigset_t signal_mask; sigemptyset(&signal_mask);
	sigaddset(&signal_mask, SIGINT);
	pthread_sigmask(SIG_BLOCK, &signal_mask, NULL);
	{ // <- Spawn threads inside this block
		assert(dbgcon_setup(io));
		assert(tty_setup(io, &cpu, options.port, options.ttys));
		if(options.show_freq) pthread_create(
			&cleanup_registry.freq, NULL,
			freq_counter_thread, &cpu.step_count
		);
	}
	// Revert the mask so that main thread gets to handle the signals
	pthread_sigmask(SIG_UNBLOCK, &signal_mask, NULL);

	// After cpu handle has been distributed, init and sendoff
	cpu_new(&cpu, mem, io, true);
	while(is_running()) {
		cpu_execute(&cpu);
		if(options.verbose)
			cpu_print_state(&cpu);
	}

	fprintf(stderr,
		"CPU halted after %lu step(s).\n",
		cpu.step_count
	);

	exit(EXIT_SUCCESS);
}
