#include <stdio.h>

#include <assert.h>
#include <stdlib.h>

#include "core/cpu.h"
#include "core/mem.h"

int main(int argc, char **argv) {
	// Make sure the memory sizes union is packed correctly
	assert(sizeof(mem_datum_t) == sizeof(uint32_t));
	assert(argc == 1 || argc == 2);

	mem_t memory = mem_new(argc == 1 ? NULL : argv[1]);
	cpu_t processor; cpu_reset(&processor, memory);
	
	uint64_t count = 0;
	for(; cpu_execute(&processor); count++);
	printf("CPU halted after %lu steps.\n", count);

	return EXIT_SUCCESS;
}
