#ifndef PPLUSEMU_CORE_MEM_H
#define PPLUSEMU_CORE_MEM_H

#include <stdint.h>

#define MEM_SIZE_KIB 1

typedef union mem_datum *mem_t;

mem_t mem_new(const char *fname);
void mem_free(mem_t memory);

uint8_t mem_fetch_byte(mem_t memory, uint32_t address);
uint16_t mem_fetch_half(mem_t memory, uint32_t address);
uint32_t mem_fetch_word(mem_t memory, uint32_t address);
void mem_store_byte(mem_t memory, uint32_t address, uint8_t byte);
void mem_store_half(mem_t memory, uint32_t address, uint16_t half);
void mem_store_word(mem_t memory, uint32_t address, uint32_t word);

#endif // PPLUSEMU_CORE_MEM_H
