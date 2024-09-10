#ifndef PPLUSEMU_UTIL_H
#define PPLUSEMU_UTIL_H

#include <stdint.h>

#include "core/cpu.h"

void hex_string_of_length(char *buffer, uint32_t value, int length);
void print_cpu_state(uint64_t cycle, cpu_t *cpu);

#endif // PPLUSEMU_UTIL_H
