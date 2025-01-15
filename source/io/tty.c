#include "tty.h"

#include <stdio.h>

static struct {
	volatile bool init;
} state = {0};

bool tty_setup(io_t io) {
	if(state.init) return false;

	(void) io;
	printf("TTY Setup!\n");

	state.init = true;
	return true;
}

bool tty_close(io_t io) {
	if(!state.init) return false;

	(void) io;
	printf("TTY Close!\n");

	state.init = false;
	return true;
}
