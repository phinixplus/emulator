#include <stdio.h>

#include <assert.h>
#include <stdlib.h>

#include "util.h"
#include "core/cpu.h"
#include "core/mem.h"

int main(int argc, char **argv) {
	// Make sure the memory sizes and instructions unions are packed correctly
	assert(sizeof(mem_datum_t) == sizeof(uint32_t));
	assert(sizeof(instruction_t) == sizeof(uint32_t));

	assert(argc == 2);

	mem_t memory = mem_new(argv[1]);
	cpu_t processor; cpu_reset(&processor, memory);

	uint64_t count = 0;
	for(; cpu_execute(&processor); count++) {
		char value[9] = {0};
		printf("---- Step: %lu ----\n", count + 1);
		hex_string_of_length(value, processor.ip, 8);
		printf("ip: %8s ", value);
		hex_string_of_length(value, processor.cond, 2);
		printf("c*: %2s\n", value);
		for(int i = 0; i < 16; i++) {
			hex_string_of_length(value, processor.data[i], 8);
			printf("x%x: %8s", i, value);
			putchar((i & 3) == 3 ? '\n' : ' ');
		}
		for(int i = 0; i < 16; i++) {
			hex_string_of_length(value, processor.addr[i], 8);
			printf("y%x: %8s", i, value);
			putchar((i & 3) == 3 ? '\n' : ' ');
		}
	}
	fprintf(stderr, "CPU halted after %lu step(s).\n", count);

	return EXIT_SUCCESS;
}
