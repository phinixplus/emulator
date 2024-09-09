#include <stdio.h>

#include <assert.h>
#include <stdlib.h>

#include "util.h"
#include "core/cpu.h"
#include "core/mem.h"
#include "core/io.h"

void dbgcon_callback(bool write, uint32_t *value) {
	assert(write);
	printf("%d\n", *value);
}

int main(int argc, char **argv) {
	assert(argc == 2);

	mem_t mem = mem_new(argv[1]);
	io_t io = io_new();
	assert(io_register_write(io, 0, dbgcon_callback) == NULL);
	cpu_t cpu; cpu_reset(&cpu, mem, io);

	uint64_t count = 0;
	for(; cpu_execute(&cpu); count++) {
		char value[9] = {0};
		printf("---- Step: %lu ----\n", count + 1);
		hex_string_of_length(value, cpu.ip, 8);
		printf("ip: %8s ", value);
		hex_string_of_length(value, cpu.cond, 2);
		printf("c*: %2s\n", value);
		for(int i = 0; i < 16; i++) {
			hex_string_of_length(value, cpu.data[i], 8);
			printf("x%x: %8s", i, value);
			putchar((i & 3) == 3 ? '\n' : ' ');
		}
		for(int i = 0; i < 16; i++) {
			hex_string_of_length(value, cpu.addr[i], 8);
			printf("y%x: %8s", i, value);
			putchar((i & 3) == 3 ? '\n' : ' ');
		}
	}
	fprintf(stderr, "CPU halted after %lu step(s).\n", count);

	return EXIT_SUCCESS;
}
