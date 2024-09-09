#ifndef PPLUSEMU_CORE_IO_H
#define PPLUSEMU_CORE_IO_H

#include <stdint.h>
#include <stdbool.h>

#define IO_ADDR_BITS 8

typedef void (*io_callback_t)(bool write, uint32_t *value);
typedef struct io_registry *io_t;

io_t io_new(void);
void io_free(io_t io);

io_callback_t io_register_read(io_t io, uint16_t port, io_callback_t callback);
io_callback_t io_register_write(io_t io, uint16_t port, io_callback_t callback);

uint32_t io_read(io_t io, uint16_t port);
void io_write(io_t io, uint16_t port, uint32_t value);

#endif // PPLUSEMU_CORE_IO_H
