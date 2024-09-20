#include "cpu.h"

#include "util.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

#pragma pack(push, 1)
typedef union instruction {
	uint32_t instr_word;
	struct {
		uint8_t opcode;
		union {
			struct {
				uint8_t funct;
				uint16_t _padding;
			} hs;
			struct {
				unsigned src_g : 4;
				unsigned dst_g : 4;
				uint16_t _padding;
			} hgg;
			struct {
				unsigned tgt_c : 3;
				unsigned neg : 1;
				unsigned tgt_g : 4;
				uint16_t _padding;
			} hcg;
			struct {
				unsigned imm : 4;
				unsigned tgt_g : 4;
				uint16_t _padding;
			} hgi;
			struct {
				unsigned src_g : 4;
				unsigned dst_g : 4;
				uint16_t imm;
			} wggi;
			struct {
				unsigned tgt_c : 3;
				unsigned neg : 1;
				unsigned tgt_g : 4;
				uint16_t imm;
			} wcgi;
		};
	};
} instruction_t;
#pragma pack(pop)

const char datareg_conv[][3] = {
	"zr", "at", "rp", "t0", "t1", "t2", "a0", "a1",
	"a2", "s0", "s1", "s2", "s3", "s4", "s5", "fp"
};

const char addrreg_conv[][3] = {
	"a3", "a4", "a5", "t3", "t4", "t5", "t6", "t7",
	"s6", "s7", "gp", "sp", "k0", "k1", "k2", "k3"
};

void cpu_reset(cpu_t *cpu, mem_t mem, io_t io) {
	// Make sure the instruction formats union is packed correctly.
	assert(sizeof(instruction_t) == sizeof(uint32_t));

	cpu->steps = 0;
	cpu->ip = 0, cpu->cond &= ~1;
	for(unsigned i = 0; i<16; i++)
		cpu->data[i] = cpu->addr[i] = 0;
	if(mem != NULL) cpu->mem = mem;
	if(io != NULL) cpu->io = io;
}

bool cpu_execute(cpu_t *cpu) {
	uint32_t inst_word = mem_fetch_word(cpu->mem, cpu->ip);
	instruction_t instr = {inst_word};

	bool tmpc1, tmpc2;
	uint32_t tmpg1, tmpg2;
	char hexbuf[9] = {0};
	switch(instr.opcode) {
		case 0x00: // PAD, HLT
			if(instr.hs.funct == 0xFF)
				return cpu->ip += 2, cpu_execute(cpu);
			else return false;
		case 0x01: // MOVxx
			cpu->data[instr.hgg.dst_g] = cpu->data[instr.hgg.src_g];
			cpu->ip += 2;
			break;
		case 0x02: // MOVxy
			cpu->data[instr.hgg.dst_g] = cpu->addr[instr.hgg.src_g];
			cpu->ip += 2;
			break;
		case 0x03: // MOVyx
			cpu->addr[instr.hgg.dst_g] = cpu->data[instr.hgg.src_g];
			cpu->ip += 2;
			break;
		case 0x04: // MOVyy
			cpu->addr[instr.hgg.dst_g] = cpu->addr[instr.hgg.src_g];
			cpu->ip += 2;
			break;
		case 0x05: // ADDrx
			cpu->data[instr.hgg.dst_g] += cpu->data[instr.hgg.src_g];
			cpu->ip += 2;
			break;
		case 0x06: // ADDry
			cpu->addr[instr.hgg.dst_g] += cpu->addr[instr.hgg.src_g];
			cpu->ip += 2;
			break;
		case 0x07: // SUBrx
			cpu->data[instr.hgg.dst_g] -= cpu->data[instr.hgg.src_g];
			cpu->ip += 2;
			break;
		case 0x08: // SUBry
			cpu->addr[instr.hgg.dst_g] -= cpu->addr[instr.hgg.src_g];
			cpu->ip += 2;
			break;
		case 0x09: // ANDrx
			cpu->data[instr.hgg.dst_g] &= cpu->data[instr.hgg.src_g];
			cpu->ip += 2;
			break;
		case 0x0A: // NNDrx
			tmpg1 = cpu->data[instr.hgg.dst_g] & cpu->data[instr.hgg.src_g];
			cpu->data[instr.hgg.dst_g] = ~tmpg1;
			cpu->ip += 2;
			break;
		case 0x0B: // IORrx
			cpu->data[instr.hgg.dst_g] |= cpu->data[instr.hgg.src_g];
			cpu->ip += 2;
			break;
		case 0x0C: // NORrx
			tmpg1 = cpu->data[instr.hgg.dst_g] | cpu->data[instr.hgg.src_g];
			cpu->data[instr.hgg.dst_g] = ~tmpg1;
			cpu->ip += 2;
			break;
		case 0x0D: // XORrx
			cpu->data[instr.hgg.dst_g] ^= cpu->data[instr.hgg.src_g];
			cpu->ip += 2;
			break;
		case 0x0E: // SLx
			cpu->data[instr.hgg.dst_g] = cpu->data[instr.hgg.src_g] << 1;
			cpu->ip += 2;
			break;
		case 0x0F: // SRUx
			cpu->data[instr.hgg.dst_g] = cpu->data[instr.hgg.src_g] >> 1;
			cpu->ip += 2;
			break;
		case 0x10: // SRSx
			tmpg1 = (uint32_t)(((int32_t)cpu->data[instr.hgg.src_g]) >> 1);
			cpu->data[instr.hgg.dst_g] = tmpg1;
			cpu->ip += 2;
			break;
		case 0x11: // BLrx
			cpu->data[instr.hgg.dst_g] <<= cpu->data[instr.hgg.src_g] & 0x1F;
			cpu->ip += 2;
			break;
		case 0x12: // BRUrx
			cpu->data[instr.hgg.dst_g] >>= cpu->data[instr.hgg.src_g] & 0x1F;
			cpu->ip += 2;
			break;
		case 0x13: // BRSrx
			tmpg1 = cpu->data[instr.hgg.src_g] & 0x1F;
			tmpg2 = (uint32_t)(((int32_t)cpu->data[instr.hgg.dst_g]) >> tmpg1);
			cpu->data[instr.hgg.dst_g] = tmpg2;
			cpu->ip += 2;
			break;
		case 0x14: // LBUrx
			tmpg1 = mem_fetch_byte(cpu->mem, cpu->data[instr.hgg.src_g]);
			cpu->data[instr.hgg.dst_g] = tmpg1;
			cpu->ip += 2;
			break;
		case 0x15: // LBUry
			tmpg1 = mem_fetch_byte(cpu->mem, cpu->addr[instr.hgg.src_g]);
			cpu->data[instr.hgg.dst_g] = tmpg1;
			cpu->ip += 2;
			break;
		case 0x16: // LBSrx
			tmpg1 = mem_fetch_byte(cpu->mem, cpu->data[instr.hgg.src_g]);
			tmpg1 |= (tmpg1 & 1 << 7) ? 0xFFFFFF00 : 0;
			cpu->data[instr.hgg.dst_g] = tmpg1;
			cpu->ip += 2;
			break;
		case 0x17: // LBSry
			tmpg1 = mem_fetch_byte(cpu->mem, cpu->addr[instr.hgg.src_g]);
			tmpg1 |= (tmpg1 & 1 << 7) ? 0xFFFFFF00 : 0;
			cpu->data[instr.hgg.dst_g] = tmpg1;
			cpu->ip += 2;
			break;
		case 0x18: // SBrx
			mem_store_byte(
				cpu->mem,
				cpu->data[instr.hgg.src_g],
				(uint8_t)(cpu->data[instr.hgg.dst_g] & 0x000000FF)
			);
			cpu->ip += 2;
			break;
		case 0x19: // SBry
			mem_store_byte(
				cpu->mem,
				cpu->addr[instr.hgg.src_g],
				(uint8_t)(cpu->data[instr.hgg.dst_g] & 0x000000FF)
			);
			cpu->ip += 2;
			break;
		case 0x1A: // LHUrx
			tmpg1 = mem_fetch_half(cpu->mem, cpu->data[instr.hgg.src_g]);
			cpu->data[instr.hgg.dst_g] = tmpg1;
			cpu->ip += 2;
			break;
		case 0x1B: // LHUry
			tmpg1 = mem_fetch_half(cpu->mem, cpu->addr[instr.hgg.src_g]);
			cpu->data[instr.hgg.dst_g] = tmpg1;
			cpu->ip += 2;
			break;
		case 0x1C: // LHSrx
			tmpg1 = mem_fetch_half(cpu->mem, cpu->data[instr.hgg.src_g]);
			tmpg1 |= (tmpg1 & 1 << 7) ? 0xFFFFFF00 : 0;
			cpu->data[instr.hgg.dst_g] = tmpg1;
			cpu->ip += 2;
			break;
		case 0x1D: // LHSry
			tmpg1 = mem_fetch_half(cpu->mem, cpu->addr[instr.hgg.src_g]);
			tmpg1 |= (tmpg1 & 1 << 7) ? 0xFFFFFF00 : 0;
			cpu->data[instr.hgg.dst_g] = tmpg1;
			cpu->ip += 2;
			break;
		case 0x1E: // SHrx
			mem_store_half(
				cpu->mem,
				cpu->data[instr.hgg.src_g],
				(uint16_t)(cpu->data[instr.hgg.dst_g] & 0x0000FFFF)
			);
			cpu->ip += 2;
			break;
		case 0x1F: // SHry
			mem_store_half(
				cpu->mem,
				cpu->addr[instr.hgg.src_g],
				(uint16_t)(cpu->data[instr.hgg.dst_g] & 0x0000FFFF)
			);
			cpu->ip += 2;
			break;
		case 0x20: // LWrx
			tmpg1 = mem_fetch_word(cpu->mem, cpu->data[instr.hgg.src_g]);
			cpu->data[instr.hgg.dst_g] = tmpg1;
			cpu->ip += 2;
			break;
		case 0x21: // LWry
			tmpg1 = mem_fetch_word(cpu->mem, cpu->addr[instr.hgg.src_g]);
			cpu->data[instr.hgg.dst_g] = tmpg1;
			cpu->ip += 2;
			break;
		case 0x22: // SWrx
			mem_store_word(
				cpu->mem,
				cpu->data[instr.hgg.src_g],
				cpu->data[instr.hgg.dst_g]
			);
			cpu->ip += 2;
			break;
		case 0x23: // SWry
			mem_store_word(
				cpu->mem,
				cpu->addr[instr.hgg.src_g],
				cpu->data[instr.hgg.dst_g]
			);
			cpu->ip += 2;
			break;
		case 0x24: // JNLrxx
			cpu->data[instr.hgg.dst_g] = cpu->ip + 2;
			cpu->ip = cpu->data[instr.hgg.src_g];
			break;
		case 0x25: // JNLrxy
			cpu->data[instr.hgg.dst_g] = cpu->ip + 2;
			cpu->ip = cpu->addr[instr.hgg.src_g];
			break;
		case 0x26: // JNLryx
			cpu->addr[instr.hgg.dst_g] = cpu->ip + 2;
			cpu->ip = cpu->data[instr.hgg.src_g];
			break;
		case 0x27: // JNLryy
			cpu->addr[instr.hgg.dst_g] = cpu->ip + 2;
			cpu->ip = cpu->addr[instr.hgg.src_g];
			break;
		case 0x28: // TZx
			tmpc1 = (instr.hcg.neg == 1);
			tmpc2 = (cpu->data[instr.hcg.tgt_g] == 0);
			if(tmpc1 != tmpc2) cpu->cond |= 1 << instr.hcg.tgt_c;
			else cpu->cond &= ~(1 << instr.hcg.tgt_c);
			cpu->ip += 2;
			break;
		case 0x29: // TZy
			tmpc1 = (instr.hcg.neg == 1);
			tmpc2 = (cpu->addr[instr.hcg.tgt_g] == 0);
			if(tmpc1 != tmpc2) cpu->cond |= 1 << instr.hcg.tgt_c;
			else cpu->cond &= ~(1 << instr.hcg.tgt_c);
			cpu->ip += 2;
			break;
		case 0x2A: // TNx
			tmpc1 = (instr.hcg.neg == 1);
			tmpc2 = ((cpu->data[instr.hcg.tgt_g] & 1 << 31) != 0);
			if(tmpc1 != tmpc2) cpu->cond |= 1 << instr.hcg.tgt_c;
			else cpu->cond &= ~(1 << instr.hcg.tgt_c);
			cpu->ip += 2;
			break;
		case 0x2B: // TNy
			tmpc1 = (instr.hcg.neg == 1);
			tmpc2 = ((cpu->addr[instr.hcg.tgt_g] & 1 << 31) != 0);
			if(tmpc1 != tmpc2) cpu->cond |= 1 << instr.hcg.tgt_c;
			else cpu->cond &= ~(1 << instr.hcg.tgt_c);
			cpu->ip += 2;
			break;
		case 0x2C: // TOx
			tmpc1 = (instr.hcg.neg == 1);
			tmpc2 = ((cpu->data[instr.hcg.tgt_g] & 1) != 0);
			if(tmpc1 != tmpc2) cpu->cond |= 1 << instr.hcg.tgt_c;
			else cpu->cond &= ~(1 << instr.hcg.tgt_c);
			cpu->ip += 2;
			break;
		case 0x2D: // TOy
			tmpc1 = (instr.hcg.neg == 1);
			tmpc2 = ((cpu->addr[instr.hcg.tgt_g] & 1) != 0);
			if(tmpc1 != tmpc2) cpu->cond |= 1 << instr.hcg.tgt_c;
			else cpu->cond &= ~(1 << instr.hcg.tgt_c);
			cpu->ip += 2;
			break;
		case 0x2E: // JCArx
			tmpc1 = (instr.hcg.neg == 1);
			tmpc2 = ((cpu->cond & 1 << instr.hcg.tgt_c) != 0);
			if(tmpc1 != tmpc2) cpu->ip = cpu->data[instr.hcg.tgt_g];
			else cpu->ip += 2;
			break;
		case 0x2F: // JCAry
			tmpc1 = (instr.hcg.neg == 1);
			tmpc2 = ((cpu->cond & 1 << instr.hcg.tgt_c) != 0);
			if(tmpc1 != tmpc2) cpu->ip = cpu->addr[instr.hcg.tgt_g];
			else cpu->ip += 2;
			break;
		case 0x30: // JCRrx
			tmpc1 = (instr.hcg.neg == 1);
			tmpc2 = ((cpu->cond & 1 << instr.hcg.tgt_c) != 0);
			if(tmpc1 != tmpc2) cpu->ip += cpu->data[instr.hcg.tgt_g];
			else cpu->ip += 2;
			break;
		case 0x31: // JCRry
			tmpc1 = (instr.hcg.neg == 1);
			tmpc2 = ((cpu->cond & 1 << instr.hcg.tgt_c) != 0);
			if(tmpc1 != tmpc2) cpu->ip += cpu->addr[instr.hcg.tgt_g];
			else cpu->ip += 2;
			break;
		case 0x32: // ADDsx
			cpu->data[instr.hgi.tgt_g] += instr.hgi.imm + 1;
			cpu->ip += 2;
			break;
		case 0x33: // ADDsy
			cpu->addr[instr.hgi.tgt_g] += instr.hgi.imm + 1;
			cpu->ip += 2;
			break;
		case 0x34: // SUBsx
			cpu->data[instr.hgi.tgt_g] -= instr.hgi.imm + 1;
			cpu->ip += 2;
			break;
		case 0x35: // SUBsy
			cpu->addr[instr.hgi.tgt_g] -= instr.hgi.imm + 1;
			cpu->ip += 2;
			break;
		case 0x36: // BLs
			cpu->data[instr.hgi.tgt_g] <<= instr.hgi.imm + 2;
			cpu->ip += 2;
			break;
		case 0x37: // BRUs
			cpu->data[instr.hgi.tgt_g] >>= instr.hgi.imm + 2;
			cpu->ip += 2;
			break;
		case 0x38: // BRSs
			tmpg1 = cpu->data[instr.hgi.tgt_g];
			tmpg2 = (uint32_t)(((int32_t)tmpg1) >> (instr.hgi.imm + 2));
			cpu->data[instr.hgi.tgt_g] = tmpg2;
			cpu->ip += 2;
			break;
		case 0x39: // ADDlx
			tmpg1 = (uint32_t)instr.wggi.imm;
			if((tmpg1 & (1 << 15)) != 0) tmpg1 |= 0xFFFF0000;
			tmpg1 += cpu->data[instr.wggi.src_g];
			cpu->data[instr.wggi.dst_g] = tmpg1;
			cpu->ip += 4;
			break;
		case 0x3A: // ADDly
			tmpg1 = (uint32_t)instr.wggi.imm;
			if((tmpg1 & (1 << 15)) != 0) tmpg1 |= 0xFFFF0000;
			tmpg1 += cpu->data[instr.wggi.src_g];
			cpu->addr[instr.wggi.dst_g] = tmpg1;
			cpu->ip += 4;
			break;
		case 0x3B: // ADDhx
			tmpg1 = ((uint32_t)instr.wggi.imm) << 16;
			tmpg1 += cpu->data[instr.wggi.src_g];
			cpu->data[instr.wggi.dst_g] = tmpg1;
			cpu->ip += 4;
			break;
		case 0x3C: // ADDhy
			tmpg1 = ((uint32_t)instr.wggi.imm) << 16;
			tmpg1 += cpu->addr[instr.wggi.src_g];
			cpu->addr[instr.wggi.dst_g] = tmpg1;
			cpu->ip += 4;
			break;
		case 0x3D: // ANDl
			tmpg1 = (uint32_t)instr.wggi.imm;
			tmpg1 &= cpu->data[instr.wggi.src_g];
			cpu->data[instr.wggi.dst_g] = tmpg1;
			cpu->ip += 4;
			break;
		case 0x3E: // ANDh
			tmpg1 = (((uint32_t)instr.wggi.imm) << 16) | 0xFFFF;
			tmpg1 &= cpu->data[instr.wggi.src_g];
			cpu->data[instr.wggi.dst_g] = tmpg1;
			cpu->ip += 4;
			break;
		case 0x3F: // IORl
			tmpg1 = (uint32_t)instr.wggi.imm;
			tmpg1 |= cpu->data[instr.wggi.src_g];
			cpu->data[instr.wggi.dst_g] = tmpg1;
			cpu->ip += 4;
			break;
		case 0x40: // IORh
			tmpg1 = ((uint32_t)instr.wggi.imm) << 16;
			tmpg1 |= cpu->data[instr.wggi.src_g];
			cpu->data[instr.wggi.dst_g] = tmpg1;
			cpu->ip += 4;
			break;
		case 0x41: // XORl
			tmpg1 = (uint32_t)instr.wggi.imm;
			tmpg1 ^= cpu->data[instr.wggi.src_g];
			cpu->data[instr.wggi.dst_g] = tmpg1;
			cpu->ip += 4;
			break;
		case 0x42: // XORh
			tmpg1 = ((uint32_t)instr.wggi.imm) << 16;
			tmpg1 ^= cpu->data[instr.wggi.src_g];
			cpu->data[instr.wggi.dst_g] = tmpg1;
			cpu->ip += 4;
			break;
		case 0x43: // IN
			tmpg1 = (uint32_t)instr.wggi.imm;
			tmpg1 += cpu->data[instr.wggi.src_g];
			tmpg1 &= (1 << IO_ADDR_BITS) - 1;
			cpu->data[instr.wggi.dst_g] = io_read(cpu->io, tmpg1);
			cpu->ip += 4;
			break;
		case 0x44: // OUT
			tmpg1 = (uint32_t)instr.wggi.imm;
			tmpg1 += cpu->data[instr.wggi.src_g];
			tmpg1 &= (1 << IO_ADDR_BITS) - 1;
			io_write(cpu->io, tmpg1, cpu->data[instr.wggi.dst_g]);
			cpu->ip += 4;
			break;
		case 0x45: // JCArx
			tmpc1 = (instr.wcgi.neg == 1);
			tmpc2 = ((cpu->cond & 1 << instr.wcgi.tgt_c) != 0);
			tmpg1 = (uint32_t)instr.wcgi.imm;
			if((tmpg1 & (1 << 15)) != 0) tmpg1 |= 0xFFFF0000;
			tmpg1 += cpu->data[instr.wcgi.tgt_g];
			if(tmpc1 != tmpc2) cpu->ip = tmpg1;
			else cpu->ip += 2;
			break;
		case 0x46: // JCAry
			tmpc1 = (instr.wcgi.neg == 1);
			tmpc2 = ((cpu->cond & 1 << instr.wcgi.tgt_c) != 0);
			tmpg1 = (uint32_t)instr.wcgi.imm;
			if((tmpg1 & (1 << 15)) != 0) tmpg1 |= 0xFFFF0000;
			tmpg1 += cpu->addr[instr.wcgi.tgt_g];
			if(tmpc1 != tmpc2) cpu->ip = tmpg1;
			else cpu->ip += 2;
			break;
		case 0x47: // JCRrx
			tmpc1 = (instr.wcgi.neg == 1);
			tmpc2 = ((cpu->cond & 1 << instr.wcgi.tgt_c) != 0);
			tmpg1 = (uint32_t)instr.wcgi.imm;
			if((tmpg1 & (1 << 15)) != 0) tmpg1 |= 0xFFFF0000;
			tmpg1 += cpu->data[instr.wcgi.tgt_g];
			if(tmpc1 != tmpc2) cpu->ip += tmpg1;
			else cpu->ip += 2;
			break;
		case 0x48: // JCRry
			tmpc1 = (instr.wcgi.neg == 1);
			tmpc2 = ((cpu->cond & 1 << instr.wcgi.tgt_c) != 0);
			tmpg1 = (uint32_t)instr.wcgi.imm;
			if((tmpg1 & (1 << 15)) != 0) tmpg1 |= 0xFFFF0000;
			tmpg1 += cpu->addr[instr.wcgi.tgt_g];
			if(tmpc1 != tmpc2) cpu->ip += tmpg1;
			else cpu->ip += 2;
			break;
		case 0x49: case 0x4A: case 0x4B:
		case 0x4C: case 0x4D: case 0x4E: case 0x4F:
		case 0x50: case 0x51: case 0x52: case 0x53:
		case 0x54: case 0x55: case 0x56: case 0x57:
		case 0x58: case 0x59: case 0x5A: case 0x5B:
		case 0x5C: case 0x5D: case 0x5E: case 0x5F:
		case 0x60: case 0x61: case 0x62: case 0x63:
		case 0x64: case 0x65: case 0x66: case 0x67:
		case 0x68: case 0x69: case 0x6A: case 0x6B:
		case 0x6C: case 0x6D: case 0x6E: case 0x6F:
		case 0x70: case 0x71: case 0x72: case 0x73:
		case 0x74: case 0x75: case 0x76: case 0x77:
		case 0x78: case 0x79: case 0x7A: case 0x7B:
		case 0x7C: case 0x7D: case 0x7E: case 0x7F:
		case 0x80: case 0x81: case 0x82: case 0x83:
		case 0x84: case 0x85: case 0x86: case 0x87:
		case 0x88: case 0x89: case 0x8A: case 0x8B:
		case 0x8C: case 0x8D: case 0x8E: case 0x8F:
		case 0x90: case 0x91: case 0x92: case 0x93:
		case 0x94: case 0x95: case 0x96: case 0x97:
		case 0x98: case 0x99: case 0x9A: case 0x9B:
		case 0x9C: case 0x9D: case 0x9E: case 0x9F:
		case 0xA0: case 0xA1: case 0xA2: case 0xA3:
		case 0xA4: case 0xA5: case 0xA6: case 0xA7:
		case 0xA8: case 0xA9: case 0xAA: case 0xAB:
		case 0xAC: case 0xAD: case 0xAE: case 0xAF:
		case 0xB0: case 0xB1: case 0xB2: case 0xB3:
		case 0xB4: case 0xB5: case 0xB6: case 0xB7:
		case 0xB8: case 0xB9: case 0xBA: case 0xBB:
		case 0xBC: case 0xBD: case 0xBE: case 0xBF:
		case 0xC0: case 0xC1: case 0xC2: case 0xC3:
		case 0xC4: case 0xC5: case 0xC6: case 0xC7:
		case 0xC8: case 0xC9: case 0xCA: case 0xCB:
		case 0xCC: case 0xCD: case 0xCE: case 0xCF:
		case 0xD0: case 0xD1: case 0xD2: case 0xD3:
		case 0xD4: case 0xD5: case 0xD6: case 0xD7:
		case 0xD8: case 0xD9: case 0xDA: case 0xDB:
		case 0xDC: case 0xDD: case 0xDE: case 0xDF:
		case 0xE0: case 0xE1: case 0xE2: case 0xE3:
		case 0xE4: case 0xE5: case 0xE6: case 0xE7:
		case 0xE8: case 0xE9: case 0xEA: case 0xEB:
		case 0xEC: case 0xED: case 0xEE: case 0xEF:
		case 0xF0: case 0xF1: case 0xF2: case 0xF3:
		case 0xF4: case 0xF5: case 0xF6: case 0xF7:
		case 0xF8: case 0xF9: case 0xFA: case 0xFB:
		case 0xFC: case 0xFD: case 0xFE: case 0xFF:
			hex_string_of_length(hexbuf, cpu->ip, 8);
			fprintf(stderr,
				"Encountered invalid opcode %xh at %8sh.\n",
				instr.opcode, hexbuf
			);
			return false;
	}
	cpu->cond &= 0xFE;
	cpu->data[0] = 0;
	cpu->steps++;
	return true;
}

void cpu_print_state(cpu_t *cpu) {
	char value[9] = {0};
	fprintf(stderr, "---- Step: %lu ----\n", cpu->steps);
	hex_string_of_length(value, cpu->ip, 8);
	fprintf(stderr, "ip: %8s ", value);
	hex_string_of_length(value, cpu->cond, 2);
	fprintf(stderr, "c*: %2s\n", value);
	for(int i = 0; i < 16; i++) {
		hex_string_of_length(value, cpu->data[i], 8);
		fprintf(stderr, "%s(x%x): %8s ", datareg_conv[i], i, value);
		putchar((i & 3) == 3 ? '\n' : ' ');
	}
	for(int i = 0; i < 16; i++) {
		hex_string_of_length(value, cpu->addr[i], 8);
		fprintf(stderr, "%s(y%x): %8s ", addrreg_conv[i], i, value);
		putchar((i & 3) == 3 ? '\n' : ' ');
	}
	fprintf(stderr, "-----------------\n");
}
