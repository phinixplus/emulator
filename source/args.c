#include "args.h"

#include <stdio.h>
#include <stdlib.h>

#include <getopt.h>

#include "io/tty.h"

void args_print_help(char *exe_name, bool verbose) {
	if(!verbose) fprintf(stderr,
	"Usage: %s [-vfh] [-p port] [-t 1-%u] <memfile>\n"
	"Run with -h for more info. \n",
	exe_name, TTY_MAX_CLIENTS);
	else fprintf(stderr,
	"PHINIX+ System Emulator\n"
	"-----------------------\n"
	"Usage: %s <options> <memfile>\n"
	"Options:\n"
	"	-v: Verbose; Output CPU state for each instruction.\n"
	"	-f: Show Frequency; Output execution speed each second.\n"
	"	-h: Extra Help; Prints this message.\n"
	"	-p: TTY Server Port; The TCP port on which to listen.\n"
	"	    Required Argument: Number between 0 and 65535.\n"
	"	-t: TTY Max Clients; Reject excess concurrent clients.\n"
	"	    Required Argument: Number between 1 and %u.\n"
	"Memory File:\n"
	"	A hexadecimal memory file in logisim8 format\n"
	"	containing the initial data for the memory.\n"
	"	The CPU starts executing at address 0.\n",
	exe_name, TTY_MAX_CLIENTS);
	exit(EXIT_FAILURE);
}

options_t args_parse(int argc, char **argv) {
	options_t options = {0};
	options.ttys = TTY_MAX_CLIENTS;
	options.port = TTY_DEFAULT_PORT;
	for(int ret; (ret = getopt(argc, argv, "+:t:p:vfh")) != -1; ) {
		switch(ret) {
			case 't':
				ret = sscanf(optarg, "%u", &options.ttys);
				if(ret == 0 || options.ttys < 1 || options.ttys > TTY_MAX_CLIENTS) {
					printf("Invalid option value: %s\n", optarg);
					args_print_help(argv[0], false);
				}
				break;
			case 'p':
				ret = sscanf(optarg, "%hu", &options.port);
				if(ret == 0) {
					printf("Invalid option value: %s\n", optarg);
					args_print_help(argv[0], false);
				}
				break;
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
