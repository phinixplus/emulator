#ifndef PPLUSEMU_IO_TTY_H
#define PPLUSEMU_IO_TTY_H

#include "core/io.h"

#include <stdbool.h>
#include <stdint.h>

#define TTY_MAX_CLIENTS 32

bool tty_setup(io_t io, uint16_t server_port);
bool tty_close(io_t io);

#endif // PPLUSEMU_IO_TTY_H
