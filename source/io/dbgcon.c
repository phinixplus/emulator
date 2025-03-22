#include "dbgcon.h"

#include <assert.h>
#include <stdatomic.h>
#include <stdio.h>
#include <time.h>

#include <pthread.h>

#include "io/portdefs.h"

static struct {
	atomic_bool is_init;
	io_fifo_t fifo;
	pthread_t thread;
} state = {0};

static void *dbgcon_thread(void *dummy) {
	(void) dummy;
	struct timespec duration = {0, 100 * 1000}; // 100us
	for(uint32_t data = 0; ; ) {
		bool success = io_fifo_read(state.fifo, &data);
		if(success) printf("%d\n", data);
		else if(!state.is_init) break;
		nanosleep(&duration, NULL);
	}
	pthread_exit(NULL);
}

void dbgcon_callback(bool rw_select, uint32_t *rw_data, void *context) {
	(void) context;
	if(rw_select) io_fifo_write(state.fifo, rw_data);
	else *rw_data = io_fifo_space_free(state.fifo);
}

bool dbgcon_setup(io_t io) {
	if(state.is_init) return false;

	state.fifo = io_fifo_new();
	assert(io_try_attach(io, IO_DBGCON, dbgcon_callback, NULL));
	pthread_create(&state.thread, NULL, dbgcon_thread, NULL);

	state.is_init = true;
	return true;
}

bool dbgcon_close(io_t io) {
	if(!state.is_init) return false;
	state.is_init = false;

	pthread_join(state.thread, NULL);
	io_fifo_del(state.fifo);
	assert(io_try_detach(io, IO_DBGCON, NULL, NULL));

	return true;
}
