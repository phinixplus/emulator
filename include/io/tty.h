#ifndef PPLUSEMU_IO_TTY_H
#define PPLUSEMU_IO_TTY_H

#include <stdbool.h>
#include <stdint.h>

#include "core/cpu.h"
#include "core/io.h"

#define TTY_MAX_CLIENTS 32
#define TTY_BUFFER_SIZE 256

bool tty_setup(io_t io, cpu_t *irq_cpu, uint16_t server_port);
bool tty_close(io_t io);

#endif // PPLUSEMU_IO_TTY_H
