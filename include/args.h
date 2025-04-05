#ifndef PPLUSEMU_ARGS_H
#define PPLUSEMU_ARGS_H

#include <stdbool.h>

typedef struct options {
	const char *file;
	unsigned ttys;
	bool verbose;
	bool show_freq;
} options_t;

void args_print_help(char *exe_name, bool verbose);
options_t args_parse(int argc, char **argv);

#endif // PPLUSEMU_ARGS_H
