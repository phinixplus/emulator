#ifndef PPLUSEMU_UTIL_H
#define PPLUSEMU_UTIL_H

#include <stdint.h>

#include "core/cpu.h"

extern const char datareg_conv[][3];
extern const char addrreg_conv[][3];

void exit_as_sighandler(int sigid);
void hex_string_of_length(char *buffer, uint32_t value, int length);
void print_cpu_state(cpu_t *cpu);

void *freq_thread(void *cycle_count);

#endif // PPLUSEMU_UTIL_H
