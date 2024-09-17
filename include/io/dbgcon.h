#ifndef PPLUSEMU_IO_DBGCON_H
#define PPLUSEMU_IO_DBGCON_H

#include "core/io.h"

#include <stdbool.h>

bool dbgcon_setup(io_t io);
bool dbgcon_close(io_t io);

#endif // PPLUSEMU_IO_DBGCON_H
