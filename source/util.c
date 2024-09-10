#include "util.h"

#include <stdio.h>
#include <string.h>

void hex_string_of_length(char *buffer, uint32_t value, int length) {
	int count = snprintf(buffer, length + 1, "%.*x", length, value);
	memset(buffer, '0', length - count);
}

void print_cpu_state(uint64_t cycle, cpu_t *cpu) {
	char value[9] = {0};
	fprintf(stderr, "---- Step: %lu ----\n", cycle);
	hex_string_of_length(value, cpu->ip, 8);
	fprintf(stderr, "ip: %8s ", value);
	hex_string_of_length(value, cpu->cond, 2);
	fprintf(stderr, "c*: %2s\n", value);
	for(int i = 0; i < 16; i++) {
		hex_string_of_length(value, cpu->data[i], 8);
		fprintf(stderr, "x%x: %8s", i, value);
		putchar((i & 3) == 3 ? '\n' : ' ');
	}
	for(int i = 0; i < 16; i++) {
		hex_string_of_length(value, cpu->addr[i], 8);
		fprintf(stderr, "y%x: %8s", i, value);
		putchar((i & 3) == 3 ? '\n' : ' ');
	}
	fprintf(stderr, "-----------------\n");
}
