#include "dbgcon.h"

#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <time.h>

static struct {
	volatile bool init;
	io_fifo_t fifo;
	pthread_t thread;
} state = {0};

static void *dbgcon_thread(void *dummy) {
	(void) dummy;
	struct timespec duration = {0, 100 * 1000}; // 100us
	for(uint32_t data = 0; state.init; ) {
		bool success = io_fifo_read(state.fifo, &data);
		if(success) printf("%d\n", data);
		nanosleep(&duration, NULL);
	}
	pthread_exit(NULL);
}

void dbgcon_callback(bool rw_select, uint32_t *rw_data, void *context) {
	(void) context;
	if(rw_select) io_fifo_write(state.fifo, rw_data);
	else *rw_data = io_fifo_space(state.fifo);
}

bool dbgcon_setup(io_t io) {
	if(state.init) return false;
	state.fifo = io_fifo_new();
	state.init = io_attach_read(io, 0, dbgcon_callback, NULL);
	state.init &= io_attach_write(io, 0, dbgcon_callback, NULL);
	if(!state.init) return false;
	pthread_create(&state.thread, NULL, dbgcon_thread, NULL);
	return true;
}

bool dbgcon_close(io_t io) {
	if(!state.init) return false;
	state.init = false;
	pthread_join(state.thread, NULL);
	io_fifo_del(state.fifo);
	assert(io_detach_read(io, 0, NULL, NULL));
	assert(io_detach_write(io, 0, NULL, NULL));
	return true;
}
