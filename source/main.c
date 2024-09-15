#include <assert.h>
#include <getopt.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "util.h"
#include "core/cpu.h"
#include "core/mem.h"
#include "core/io.h"

#include "io/dbgcon.h"
#include "io/telnet.h"

static struct options {
	bool verbose;
} options;

void *freq_counter_thread(void *data) {
	uint64_t *count = (uint64_t *) data;
	uint64_t old_count = 0;
	while(true) {
		sleep(1);
		uint64_t tmp = *count;
		float freq_mhz = (tmp - old_count) / 1e6;
		fprintf(stderr, "%.1f MHz\n", freq_mhz);
		old_count = tmp;
	}
}

void exit_as_sighandler(int sigid) { 
	fprintf(stderr, "Caught signal: %s\n", strsignal(sigid));
	exit(EXIT_FAILURE);
}

void print_help(char *name) {
	fprintf(stderr,
	"Usage: %s [-v] <memfile>\n"
	"	-v: Verbose; Output CPU state for each instruction.\n",
	name);
	exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
	options.verbose = false;
	for(int ret; (ret = getopt(argc, argv, "+v")) != -1; ) {
		switch(ret) {
			case 'v': options.verbose = true; break;
			default: print_help(argv[0]);
		}
	}
	if(argc - optind != 1) print_help(argv[0]);

	signal(SIGPIPE, SIG_IGN);
	signal(SIGINT, exit_as_sighandler);

	// Setup IO
	io_t io = io_new();
	assert(dbgcon_init(io));
	assert(telnet_init(io, 2323));
	atexit(telnet_cleanup);

	// Setup memory and CPU
	mem_t mem = mem_new(argv[optind]);
	cpu_t cpu; cpu_reset(&cpu, mem, io);

	// Start frequency counter
	uint64_t count = 0;
	pthread_t freq_counter;
	if(!options.verbose) {
		pthread_create(
			&freq_counter, NULL,
			freq_counter_thread,
			&count
		);
	}

	// Do cycles and print sate if verbose
	do { if(options.verbose) print_cpu_state(count, &cpu); }
	while(count++, cpu_execute(&cpu));
	fprintf(stderr, "CPU halted after %lu step(s).\n", count);

	io_del(io);
	mem_del(mem);
	if(!options.verbose) {
		pthread_cancel(freq_counter);
		pthread_join(freq_counter, NULL);
	}
	exit(EXIT_SUCCESS);
}
