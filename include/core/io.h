#ifndef PPLUSEMU_CORE_IO_H
#define PPLUSEMU_CORE_IO_H

#include <stdint.h>
#include <stdbool.h>

#define IO_ADDR_BITS 8
#define IO_FIFO_SIZE_BITS 8

// IO Registry Section //

typedef void (*io_callback_t)(bool rw_select, uint32_t *rw_data, void *context);
typedef struct io_registry *io_t;

io_t io_new(void);
void io_del(io_t io);

bool io_attach_read(io_t io, uint16_t port, io_callback_t callback, void *context);
bool io_detach_read(io_t io, uint16_t port, io_callback_t *ret_callback, void **ret_context);
bool io_attach_write(io_t io, uint16_t port, io_callback_t callback, void *context);
bool io_detach_write(io_t io, uint16_t port, io_callback_t *ret_callback, void **ret_context);

uint32_t io_read(io_t io, uint16_t port);
void io_write(io_t io, uint16_t port, uint32_t value);

// IO FIFO Section //

typedef struct io_fifo_object *io_fifo_t;

io_fifo_t io_fifo_new(void);
void io_fifo_del(io_fifo_t fifo);

bool io_fifo_read(io_fifo_t fifo, uint32_t *data);
bool io_fifo_write(io_fifo_t fifo, uint32_t *data);
uint32_t io_fifo_space(io_fifo_t fifo);

#endif // PPLUSEMU_CORE_IO_H
