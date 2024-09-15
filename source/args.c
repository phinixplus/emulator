#include "args.h"

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

void args_print_help(char *exe_name) {
	fprintf(stderr,
	"Usage: %s [-v] <memfile>\n"
	"	-v: Verbose; Output CPU state for each instruction.\n",
	exe_name);
	exit(EXIT_FAILURE);
}

options_t args_parse(int argc, char **argv) {
	options_t options = {0};
	options.verbose = false;
	for(int ret; (ret = getopt(argc, argv, "+v")) != -1; ) {
		switch(ret) {
			case 'v': options.verbose = true; break;
			default: args_print_help(argv[0]);
		}
	}
	if(argc - optind != 1) args_print_help(argv[0]);
	options.file = argv[optind];
	return options;
}
