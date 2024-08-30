#include "mem.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
	mem_t memory = (mem_t) calloc(MEMORY_SIZE_KIB, 1024);
	if(fname == NULL) return memory;

	char buffer[8] = {0};
	FILE *file = fopen(fname, "r");
	if(fname == NULL) return mem_free(memory), NULL;

	fread(buffer, 8, sizeof(char), file);
	if(strncmp(buffer, "v2.0 raw", 8) != 0)
		return fclose(file), mem_free(memory), NULL;

	for(uint32_t i = 0; read_byte(file, &memory[i>>2].bytes[i&3]); i++);

	fclose(file);
	return memory;
}

void mem_free(mem_t memory) {
	free(memory);
}

uint8_t mem_fetch_byte(mem_t memory, uint32_t address) {
	(void) memory;
	(void) address;
	return 0;
}

uint16_t mem_fetch_half(mem_t memory, uint32_t address) {
	(void) memory;
	(void) address;
	return 0;
}

uint32_t mem_fetch_word(mem_t memory, uint32_t address) {
	mem_datum_t data = memory[address >> 2];
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
		data.bytes[0] = data.halves[1];
		data.halves[1] = tmp;
	}
	return data.word;
}

void mem_store_byte(mem_t memory, uint32_t address, uint8_t byte) {
	(void) memory;
	(void) address;
	(void) byte;
}

void mem_store_half(mem_t memory, uint32_t address, uint16_t half) {
	(void) memory;
	(void) address;
	(void) half;
}

void mem_store_word(mem_t memory, uint32_t address, uint32_t word) {
	(void) memory;
	(void) address;
	(void) word;
}
