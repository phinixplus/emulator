#ifndef PPLUSEMU_IO_DBGCON_H
#define PPLUSEMU_IO_DBGCON_H

#include <stdbool.h>

#include "core/io.h"

bool dbgcon_setup(io_t io);
bool dbgcon_close(io_t io);

#endif // PPLUSEMU_IO_DBGCON_H
