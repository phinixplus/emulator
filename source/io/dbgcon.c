#include "dbgcon.h"

#include <assert.h>
#include <stdio.h>

void dbgcon_callback(bool rw_select, uint32_t *data) {
	assert(rw_select);
	printf("%d\n", *data);
}

bool dbgcon_init(io_t io) {
	return io_attach_write(io, 0, dbgcon_callback);
}