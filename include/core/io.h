#ifndef PPLUSEMU_CORE_IO_H
#define PPLUSEMU_CORE_IO_H

#include <stdint.h>
#include <stdbool.h>

#define IO_ADDR_BITS 8

typedef void (*io_callback_t)(bool write, uint32_t *value);
typedef struct io_registry *io_t;

io_t io_new(void);
void io_free(io_t io);

bool io_attach_read(io_t io, uint16_t port, io_callback_t callback);
bool io_detach_read(io_t io, uint16_t port, io_callback_t *ret_callback);
bool io_attach_write(io_t io, uint16_t port, io_callback_t callback);
bool io_detach_write(io_t io, uint16_t port, io_callback_t *ret_callback);

uint32_t io_read(io_t io, uint16_t port);
void io_write(io_t io, uint16_t port, uint32_t value);

#endif // PPLUSEMU_CORE_IO_H
