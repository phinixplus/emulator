#include "mem.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#pragma pack(push, 1)
typedef union mem_datum {
	uint32_t word;
	uint16_t halves[2];
	uint8_t bytes[4];
} mem_datum_t;
#pragma pack(pop)

static bool read_byte(FILE *file, uint8_t *dst) {
	char c = fgetc(file);
	if(feof(file)) return false;
	while(
		c == '\n' || c == '\r' ||
		c == '\t' || c == ' '
	) c = fgetc(file);
	ungetc(c, file);

	uint8_t save = *dst;
	*dst = 0;
	for(int i=0; i<2; i++) {
		c = fgetc(file);
		if(c >= '0' && c <= '9') *dst |= ((c - '0') & 15) << (4 - 4 * i);
		else if(c >= 'A' && c <= 'F') *dst |= ((c - 'A' + 10) & 15) << (4 - 4 * i);
		else if(c >= 'a' && c <= 'f') *dst |= ((c - 'a' + 10) & 15) << (4 - 4 * i);
		else return *dst = save, false;
	}

	return true;
}

mem_t mem_new(const char *fname) {
	// Make sure the datums union is packed correctly.
	assert(sizeof(mem_datum_t) == sizeof(uint32_t));

	mem_t mem = (mem_t) calloc(MEM_SIZE_KIB, 1024);
	if(fname == NULL) return mem;

	char buffer[8] = {0};
	FILE *file = fopen(fname, "r");
	if(file == NULL) return mem_free(mem), NULL;

	size_t ret = fread(buffer, sizeof(char), 8, file);
	if(ret != 8) return mem_free(mem), NULL;
	if(strncmp(buffer, "v2.0 raw", 8) != 0)
		return fclose(file), mem_free(mem), NULL;

	for(uint32_t i = 0; read_byte(file, &mem[i>>2].bytes[i&3]); i++);

	fclose(file);
	return mem;
}

void mem_free(mem_t mem) {
	free(mem);
}

uint8_t mem_fetch_byte(mem_t mem, uint32_t address) {
	mem_datum_t data = mem[address >> 2];
	return data.bytes[address & 3];
}

uint16_t mem_fetch_half(mem_t mem, uint32_t address) {
	mem_datum_t data = mem[address >> 2];
	if(address & 1) {
		uint8_t tmp1 = data.bytes[0];
		uint8_t tmp2 = data.bytes[0];
		data.bytes[0] = data.bytes[1];
		data.bytes[2] = data.bytes[3];
		data.bytes[1] = tmp1;
		data.bytes[3] = tmp2;
	}
	return data.halves[(address & 2) ? 1 : 0];
}

uint32_t mem_fetch_word(mem_t mem, uint32_t address) {
	mem_datum_t data = mem[address >> 2];
	if(address & 1) {
		uint8_t tmp1 = data.bytes[0];
		uint8_t tmp2 = data.bytes[0];
		data.bytes[0] = data.bytes[1];
		data.bytes[2] = data.bytes[3];
		data.bytes[1] = tmp1;
		data.bytes[3] = tmp2;
	}
	if(address & 2) {
		uint16_t tmp = data.halves[0];
		data.halves[0] = data.halves[1];
		data.halves[1] = tmp;
	}
	return data.word;
}

void mem_store_byte(mem_t mem, uint32_t address, uint8_t byte) {
	mem_datum_t previous = mem[address >> 2];
	previous.bytes[address & 3] = byte;
	mem[address >> 2] = previous;
}

void mem_store_half(mem_t mem, uint32_t address, uint16_t half) {
	mem_datum_t previous =  mem[address >> 2];
	previous.halves[(address & 2) ? 1 : 0] = half;
	if(address & 1) {
		uint8_t tmp1 = previous.bytes[0];
		previous.bytes[0] = previous.bytes[1];
		previous.bytes[1] = tmp1;
	}
	mem[address >> 2] = previous;
}

void mem_store_word(mem_t mem, uint32_t address, uint32_t word) {
	mem_datum_t previous =  mem[address >> 2];
	previous.word = word;
	if(address & 1) {
		uint8_t tmp1 = previous.bytes[0];
		uint8_t tmp2 = previous.bytes[0];
		previous.bytes[0] = previous.bytes[1];
		previous.bytes[2] = previous.bytes[3];
		previous.bytes[1] = tmp1;
		previous.bytes[3] = tmp2;
	}
	if(address & 2) {
		uint16_t tmp = previous.halves[0];
		previous.halves[0] = previous.halves[1];
		previous.halves[1] = tmp;
	}
	mem[address >> 2] = previous;
}
