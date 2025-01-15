#ifndef PPLUSEMU_IO_TTY_H
#define PPLUSEMU_IO_TTY_H

#include "core/io.h"

#include <stdbool.h>

bool tty_setup(io_t io);
bool tty_close(io_t io);

#endif // PPLUSEMU_IO_TTY_H
