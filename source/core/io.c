#include "io.h"

#include "util.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

// IO Registry Section //

typedef struct io_registry {
	io_callback_t read_callbacks[(1 << IO_ADDR_BITS)];
	io_callback_t write_callbacks[(1 << IO_ADDR_BITS)];
} io_registry_t;

io_t io_new(void) {
	io_t io = calloc(1, sizeof(io_registry_t));
	return io;
}

void io_del(io_t io) {
	free(io);
}

bool io_attach_read(io_t io, uint16_t port, io_callback_t callback) {
	io_callback_t *spot = &io->read_callbacks[port];
	if(*spot != NULL) return false;
	return *spot = callback, true;
}

bool io_detach_read(io_t io, uint16_t port, io_callback_t *ret_callback) {
	io_callback_t *spot = &io->read_callbacks[port];
	if(*spot == NULL) return false;
	if(ret_callback != NULL) *ret_callback = *spot;
	return *spot = NULL, true;
}

bool io_attach_write(io_t io, uint16_t port, io_callback_t callback) {
	io_callback_t *spot = &io->write_callbacks[port];
	if(*spot != NULL) return false;
	return *spot = callback, true;
}

bool io_detach_write(io_t io, uint16_t port, io_callback_t *ret_callback) {
	io_callback_t *spot = &io->write_callbacks[port];
	if(*spot == NULL) return false;
	if(ret_callback != NULL) *ret_callback = *spot;
	return *spot = NULL, true;
}

uint32_t io_read(io_t io, uint16_t port) {
	uint32_t value = 0;
	io_callback_t callback = io->read_callbacks[port];
	if(callback == NULL) {
		char hexbuf[9] = {0};
		hex_string_of_length(hexbuf, port, (IO_ADDR_BITS - 1) / 4 + 1);
		fprintf(stderr, "Attempt to read from unassigned port %sh\n", hexbuf);
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
		fprintf(stderr, "Attempt to write to unassigned port %sh\n", hexbuf);
		return;
	}
	callback(true, &value);
}

// IO FIFO Section //

typedef struct io_fifo_object {
	pthread_mutex_t mutex;
	volatile uint32_t widx, ridx;
	volatile bool full, empty;
	volatile uint32_t buffer[];
} io_fifo_object_t;

io_fifo_t io_fifo_new(void) {
	size_t size = sizeof(io_fifo_object_t);
	size += (1 << IO_FIFO_SIZE_BITS) * sizeof(uint32_t);
	io_fifo_t fifo = (io_fifo_t) calloc(1, size);
	if(fifo == NULL) return NULL;
	if(pthread_mutex_init(&fifo->mutex, NULL) != 0) return free(fifo), NULL;
	fifo->widx = fifo->ridx = 0, fifo->empty = true, fifo->full = false;
	return fifo;
}

void io_fifo_del(io_fifo_t fifo) {
	pthread_mutex_trylock(&fifo->mutex);
	pthread_mutex_unlock(&fifo->mutex);
	pthread_mutex_destroy(&fifo->mutex);
	free(fifo);
}

bool io_fifo_read(io_fifo_t fifo, uint32_t *data) {
	if(fifo->empty) return false;
	pthread_mutex_lock(&fifo->mutex);
	*data = fifo->buffer[fifo->ridx];
	fifo->ridx = (fifo->ridx + 1) & ((1 << IO_FIFO_SIZE_BITS) - 1);
	uint32_t space = (fifo->widx - fifo->ridx) & ((1 << IO_FIFO_SIZE_BITS) - 1);
	fifo->empty = (space == 0), fifo->full = false;
	pthread_mutex_unlock(&fifo->mutex);
	return true;
}

bool io_fifo_write(io_fifo_t fifo, uint32_t *data) {
	if(fifo->full) return false;
	pthread_mutex_lock(&fifo->mutex);
	fifo->buffer[fifo->widx] = *data;
	uint32_t space = (fifo->widx - fifo->ridx) & ((1 << IO_FIFO_SIZE_BITS) - 1);
	fifo->empty = false, fifo->full = space == ((1 << IO_FIFO_SIZE_BITS) - 1);
	fifo->widx = (fifo->widx + 1) & ((1 << IO_FIFO_SIZE_BITS) - 1);
	pthread_mutex_unlock(&fifo->mutex);
	return true;
}

uint32_t io_fifo_space(io_fifo_t fifo) {
	uint32_t space = 1 << IO_FIFO_SIZE_BITS;
	return space - fifo->widx + fifo->ridx;
}
