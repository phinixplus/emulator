#ifndef PPLUSEMU_IO_TELNET_H
#define PPLUSEMU_IO_TELNET_H

#include "core/io.h"

#include <stdbool.h>
#include <stdint.h>

bool telnet_init(io_t io, uint16_t net_port);
void telnet_cleanup(void);

#endif // PPLUSEMU_IO_TELNET_H
