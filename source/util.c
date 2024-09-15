#include "util.h"

#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

const char datareg_conv[][3] = {
	"zr", "at", "rp", "t0", "t1", "t2", "a0", "a1",
	"a2", "s0", "s1", "s2", "s3", "s4", "s5", "fp"
};

const char addrreg_conv[][3] = {
	"a3", "a4", "a5", "t3", "t4", "t5", "t6", "t7",
	"s6", "s7", "gp", "sp", "k0", "k1", "k2", "k3"
};

void exit_as_sighandler(int sigid) { 
	fprintf(stderr, "Caught signal: %s\n", strsignal(sigid));
	exit(EXIT_FAILURE);
}

void hex_string_of_length(char *buffer, uint32_t value, int length) {
	int count = snprintf(buffer, length + 1, "%.*x", length, value);
	memset(buffer, '0', length - count);
}

void print_cpu_state(cpu_t *cpu) {
	char value[9] = {0};
	fprintf(stderr, "---- Step: %lu ----\n", cpu->steps);
	hex_string_of_length(value, cpu->ip, 8);
	fprintf(stderr, "ip: %8s ", value);
	hex_string_of_length(value, cpu->cond, 2);
	fprintf(stderr, "c*: %2s\n", value);
	for(int i = 0; i < 16; i++) {
		hex_string_of_length(value, cpu->data[i], 8);
		fprintf(stderr, "%s(x%x): %8s ", datareg_conv[i], i, value);
		putchar((i & 3) == 3 ? '\n' : ' ');
	}
	for(int i = 0; i < 16; i++) {
		hex_string_of_length(value, cpu->addr[i], 8);
		fprintf(stderr, "%s(y%x): %8s ", addrreg_conv[i], i, value);
		putchar((i & 3) == 3 ? '\n' : ' ');
	}
	fprintf(stderr, "-----------------\n");
}

void *freq_thread(void *cycle_count) {
	uint64_t *count = (uint64_t *) cycle_count;
	uint64_t old_count = 0;
	while(true) {
		sleep(1);
		uint64_t tmp = *count;
		float freq_mhz = (tmp - old_count) / 1e6;
		fprintf(stderr, "%.1f MHz\n", freq_mhz);
		old_count = tmp;
	}
}
