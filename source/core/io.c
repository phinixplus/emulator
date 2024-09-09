#include "io.h"

#include "util.h"

#include <stdio.h>
#include <stdlib.h>

typedef struct io_registry {
	io_callback_t read_callbacks[(1 << IO_ADDR_BITS)];
	io_callback_t write_callbacks[(1 << IO_ADDR_BITS)];
} io_registry_t;

io_t io_new(void) {
	io_t io = (io_t) calloc(1, sizeof(io_registry_t));
	return io;
}

void io_free(io_t io) {
	free(io);
}

io_callback_t io_register_read(
	io_t io, uint16_t port,
	io_callback_t callback
) {
	io_callback_t *spot = &io->read_callbacks[port];
	io_callback_t old = *spot;
	*spot = callback;
	return old;
}

io_callback_t io_register_write(
	io_t io, uint16_t port,
	io_callback_t callback
) {
	io_callback_t *spot = &io->write_callbacks[port];
	io_callback_t old = *spot;
	*spot = callback;
	return old;
}

uint32_t io_read(io_t io, uint16_t port) {
	uint32_t value = 0;
	io_callback_t callback = io->read_callbacks[port];
	if(callback == NULL) {
		char hexbuf[9] = {0};
		hex_string_of_length(hexbuf, port, (IO_ADDR_BITS - 1) / 4 + 1);
		fprintf(stderr, "Attempt to read from unassigned port %s\n", hexbuf);
		return 0;
	}
	callback(false, &value);
	return value;
}

void io_write(io_t io, uint16_t port, uint32_t value) {
	io_callback_t callback = io->write_callbacks[port];
	if(callback == NULL) {
		char hexbuf[9] = {0};
		hex_string_of_length(hexbuf, port, (IO_ADDR_BITS - 1) / 4 + 1);
		fprintf(stderr, "Attempt to write to unassigned port %s\n", hexbuf);
		return;
	}
	callback(true, &value);
}