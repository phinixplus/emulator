#include "args.h"

#include <stdio.h>
#include <stdlib.h>

#include <getopt.h>

void args_print_help(char *exe_name, bool verbose) {
	if(!verbose) fprintf(stderr,
	"Usage: %s [-vfh] <memfile>\n"
	"Run with -h for more info. \n",
	exe_name);
	else fprintf(stderr,
	"PHINIX+ System Emulator\n"
	"-----------------------\n"
	"Usage: %s <options> <memfile>\n"
	"Options:\n"
	"	-v: Verbose; Output CPU state for each instruction.\n"
	"	-f: Show Frequency; Output execution speed each second.\n"
	"	-h: Extra Help; Prints this message.\n"
	"Memory File:\n"
	"	A hexadecimal memory file in logisim8 format\n"
	"	containing the initial data for the memory.\n"
	"	The CPU starts executing at address 0.\n",
	exe_name);
	exit(EXIT_FAILURE);
}

options_t args_parse(int argc, char **argv) {
	options_t options = {0};
	for(int ret; (ret = getopt(argc, argv, "+vfh")) != -1; ) {
		switch(ret) {
			case 'v': options.verbose = true; break;
			case 'f': options.show_freq = true; break;
			case 'h': args_print_help(argv[0], true); break;
			default:  args_print_help(argv[0], false); break;
		}
	}
	if(argc - optind != 1) args_print_help(argv[0], false);
	options.file = argv[optind];
	return options;
}
