#ifndef PPLUSEMU_MAIN_H
#define PPLUSEMU_MAIN_H

#include <stdbool.h>

bool is_running(void);
void stop_running(void);

void exit_as_sighandler(int sigid);

#endif // PPLUSEMU_MAIN_H
