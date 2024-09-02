#include "util.h"

#include <stdio.h>
#include <string.h>

void hex_string_of_length(char *buffer, uint32_t value, int length) {
	int count = snprintf(buffer, length + 1, "%.*x", length, value);
	memset(buffer, '0', length - count);
}