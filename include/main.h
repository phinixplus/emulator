#ifndef PPLUSEMU_MAIN_H
#define PPLUSEMU_MAIN_H

#if __BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__
#error "Why are you trying to run this in a BE processor??"
#endif

#include <stdbool.h>

bool is_running(void);
void stop_running(void);

void exit_as_sighandler(int sigid);

#endif // PPLUSEMU_MAIN_H
