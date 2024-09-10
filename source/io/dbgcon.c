#include "dbgcon.h"

#include <assert.h>
#include <stdio.h>

void dbgcon_callback(bool write, uint32_t *value) {
	assert(write);
	printf("%d\n", *value);
}

bool dbgcon_init(io_t io) {
	return io_attach_write(io, 0, dbgcon_callback);
}
