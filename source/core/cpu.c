#include "cpu.h"

#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>

#include "main.h"

// TODO: Rewrite this to not assume bitfield ordering

#pragma pack(push, 1)
typedef union instruction {
	uint32_t opcode_word;
	struct __attribute__((packed)) {
		uint8_t opcode;
		union {
			struct instr_wgil {
				uint8_t tgt_g: 4;
				uint8_t imm20_hi: 4;
				uint8_t imm20_md: 8;
				uint8_t imm20_lo: 8;
			} wgil;
			struct instr_wggih {
				uint8_t tgt_g: 4;
				uint8_t src_g: 4;
				uint8_t imm16_hi: 8;
				uint8_t imm16_lo: 8;
			} wggih;
			struct instr_wcgih {
				uint8_t tgt_g: 4;
				uint8_t tgt_c: 3;
				uint8_t neg_tc: 1;
				uint8_t imm16_hi: 8;
				uint8_t imm16_lo: 8;
			} wcgih;
			struct instr_wcc3g {
				uint8_t dst_g: 4;
				uint8_t src1_g: 4;
				uint8_t funct: 4;
				uint8_t src2_g: 4;
				uint8_t src_c: 3;
				uint8_t neg_sc: 1;
				uint8_t tgt_c: 3;
				uint8_t neg_tc: 1;
			} wcc3g;
		};
	};
} instruction_t;
#pragma pack(pop)

void cpu_new(cpu_t *cpu, mem_t mem, io_t io) {
	// Make sure the instruction formats union is packed correctly.
	assert(sizeof(instruction_t) == sizeof(uint32_t));

	cpu->step_count = 0;
	cpu->start_addr = 0;
	pthread_mutex_init(&cpu->mutex, NULL);
	pthread_cond_init(&cpu->signal, NULL);

	cpu->ip = cpu->jp = 0;
	cpu->cond &= ~1;
	for(unsigned i = 0; i<16; i++)
		cpu->gpr[i] = 0;

	cpu->mem = mem, cpu->io = io;
	atomic_store(&cpu->ipm.is_init, false);
}

void cpu_del(cpu_t *cpu) {
	pthread_mutex_destroy(&cpu->mutex);
	pthread_cond_destroy(&cpu->signal);
}

typedef struct opcode {
	enum opcode_type {
		INVALID = 0, WGIL,
		WGGIH, WCGIH, WCC3G,
	} type;
	union {
		enum opcode_wgil {
			JNLil, JMPil
		} wgil;
		enum opcode_wggih {
			INP, OUT,
			LBSih, LBUih, SBih,
			LHSih, LHUih, SHih,
			LWSih, LWUih, SWih,
			ADDih, ANDih, IORih, XORih,
			ALUI, RLUI,
		} wggih;
		enum opcode_wcgih {
			CJNLih, CJMPih
		} wcgih;
		enum opcode_wcc3g {
			ADDr, SUBr
		} wcc3g;
	};
} opcode_t;

#define PUT_WGIL( ADDR, TYPE) [ADDR] = {.type = WGIL,  .wgil  = TYPE}
#define PUT_WGGIH(ADDR, TYPE) [ADDR] = {.type = WGGIH, .wggih = TYPE}
#define PUT_WCGIH(ADDR, TYPE) [ADDR] = {.type = WCGIH, .wcgih = TYPE}
#define PUT_WCC3G(ADDR, TYPE) [ADDR] = {.type = WCC3G, .wcc3g = TYPE}

static const opcode_t opcodes[256] = {
	PUT_WGGIH(0x10, INP),    PUT_WGGIH(0x11, OUT),

	PUT_WGGIH(0x14, LBSih),  PUT_WGGIH(0x15, LBUih),
	PUT_WGGIH(0x16, LHSih),  PUT_WGGIH(0x17, LHUih),
	PUT_WGGIH(0x18, LWSih),  PUT_WGGIH(0x19, LWUih),

	PUT_WGGIH(0x1C, SBih),   PUT_WGGIH(0x1D, SHih),
	PUT_WGGIH(0x1E, SWih),

	PUT_WGGIH(0x20, ADDih),  PUT_WGGIH(0x21, ANDih),
	PUT_WGGIH(0x22, IORih),  PUT_WGGIH(0x23, XORih),

	PUT_WGGIH(0x28, ALUI),   PUT_WGGIH(0x29, RLUI),

	PUT_WGIL( 0x30, JNLil),  PUT_WGIL( 0x31,  JMPil),
	PUT_WCGIH(0x32, CJNLih), PUT_WCGIH(0x33, CJMPih),

	PUT_WCC3G(0x40, ADDr),   PUT_WCC3G(0x41, SUBr),
};

static inline bool check_overflow(uint32_t x, uint32_t y) {
	bool neg_x = EXTRACT(x, 1, 31), neg_y = EXTRACT(y, 1, 31);
	return neg_x == neg_y && EXTRACT(x + y, 1, 31) != neg_x;
}

static inline bool check_carry(uint32_t x, uint32_t y) {
	return y > UINT32_MAX - x;
}

static inline bool check_karry(uint32_t x, uint32_t y) {
	return check_overflow(x, y) != EXTRACT(x + y, 1, 31);
}

typedef struct multi_result {
	uint32_t value;
	bool flag;
} multi_result_t;

static uint32_t execute_wgil(
	cpu_t *cpu, enum opcode_wgil type,
	uint32_t imm, uint32_t tgt_g
) {
	switch(type) {
		case JNLil: tgt_g = cpu->ip + 4, cpu->ip += imm << 1; break;
		case JMPil: cpu->ip += (imm << 1) + tgt_g; break;
	}

	return tgt_g;
}

static uint32_t execute_wggih(
	cpu_t *cpu, enum opcode_wggih type,
	uint32_t imm, uint32_t tgt_g, uint32_t src_g
) {
	switch(type) {
		case INP:   tgt_g = io_read(cpu->io, src_g + imm); break;
		case OUT:   io_write(cpu->io, src_g + imm, tgt_g); break;
		case LBSih: tgt_g = (int8_t) mem_fetch_byte(cpu->mem, imm + src_g); break;
		case LBUih: tgt_g = mem_fetch_byte(cpu->mem, imm + src_g); break;
		case SBih:  mem_store_byte(cpu->mem, imm + src_g, tgt_g); break;
		case LHSih: tgt_g = (int16_t) mem_fetch_half(cpu->mem, imm + src_g); break;
		case LHUih: tgt_g = mem_fetch_half(cpu->mem, imm + src_g); break;
		case SHih:  mem_store_half(cpu->mem, imm + src_g, tgt_g); break;
		case LWSih: /* fallthrough */
		case LWUih: tgt_g = mem_fetch_word(cpu->mem, imm + src_g); break;
		case SWih:  mem_store_word(cpu->mem, imm + src_g, tgt_g); break;
		case ADDih: tgt_g = src_g + imm; break;
		case ANDih: tgt_g = src_g & imm; break;
		case IORih: tgt_g = src_g | imm; break;
		case XORih: tgt_g = src_g ^ imm; break;
		case ALUI:  tgt_g = imm << 16 | EXTRACT(tgt_g, 16, 0); break;
		case RLUI:  tgt_g = (imm << 16 | EXTRACT(tgt_g, 16, 0)) + cpu->ip; break;
	}

	cpu->ip += 4;
	return tgt_g;
}

static uint32_t execute_wcgih(
	cpu_t *cpu, enum opcode_wcgih type,
	uint32_t imm, uint32_t tgt_g, bool tgt_c
) {
	switch(type) {
		case CJNLih:
			if(!tgt_c) cpu->ip += 4;
			else tgt_g = cpu->ip + 4, cpu->ip += imm << 1;
			break;
		case CJMPih:
			if(!tgt_c) cpu->ip += 4;
			else cpu->ip += (imm << 1) + tgt_g;
			break;
	}

	return tgt_g;
}

static multi_result_t execute_wcc3g(
	cpu_t *cpu, enum opcode_wcc3g type, uint8_t funct,
	uint32_t src1_g, uint32_t src2_g,
	bool tgt_c, bool src_c, bool neg_tc
) {
	(void) cpu, (void) src_c;
	uint32_t dst_g = 0;
	switch(type) {
		bool mix;
		case ADDr:
			dst_g = src1_g + src2_g;
			switch(EXTRACT(funct, 2, 0)) {
				case 0: tgt_c = check_carry(src1_g, src2_g); break;
				case 1: tgt_c = check_karry(src1_g, src2_g); break;
				case 2: tgt_c = check_overflow(src1_g, src2_g); break;
				case 3: break;
			}
			tgt_c = neg_tc ? !tgt_c : tgt_c;
			mix = EXTRACT(funct, 1, 2);
			mix ? (tgt_c &= src_c) : (tgt_c |= src_c);
			break;
		case SUBr:
			dst_g = src1_g - src2_g;
			switch(EXTRACT(funct, 2, 0)) {
				case 0: tgt_c = check_carry(src1_g, -src2_g); break;
				case 1: tgt_c = check_karry(src1_g, src2_g); break;
				case 2: tgt_c = check_overflow(src1_g, -src2_g); break;
				case 3: break;
			}
			tgt_c = neg_tc ? !tgt_c : tgt_c;
			mix = EXTRACT(funct, 1, 2);
			mix ? (tgt_c &= src_c) : (tgt_c |= src_c);
			break;
	}

	cpu->ip += 4;
	return (multi_result_t) {
		.value = dst_g,
		.flag = tgt_c
	};
}

// TODO: Mutexes are a huge performance bottleneck; move to IPM and use atomics

void cpu_execute(cpu_t *cpu) {
	pthread_mutex_lock(&cpu->mutex);
	if(ipm_interrupted(cpu) && !ipm_check_privilege(cpu, false)) {
		cpu->jp = cpu->ip, cpu->ip = cpu->start_addr;
		ipm_set_privilege(cpu, true);
	}
	pthread_mutex_unlock(&cpu->mutex);

	uint32_t inst_word = mem_fetch_word(cpu->mem, cpu->ip);
	instruction_t instr = {inst_word};
	opcode_t opcode = opcodes[instr.opcode];
	switch(opcode.type) {
		uint32_t imm, tgt_g, src1_g, src2_g;
		bool tgt_c, src_c;
		case WGIL:
			tgt_g = cpu->gpr[instr.wgil.tgt_g];
			imm  = instr.wgil.imm20_hi << 4;
			imm  = ((uint32_t) (int8_t) imm) << 12;
			imm |= instr.wgil.imm20_md << 8;
			imm |= instr.wgil.imm20_lo;
			tgt_g = execute_wgil(cpu, opcode.wgil, imm, tgt_g);
			cpu->gpr[instr.wggih.tgt_g] = tgt_g;
			break;
		case WGGIH:
			tgt_g = cpu->gpr[instr.wggih.tgt_g];
			src1_g  =  cpu->gpr[instr.wggih.src_g];
			imm  = instr.wggih.imm16_lo;
			imm |= instr.wggih.imm16_hi << 8;
			imm  = (uint32_t) (int16_t) imm;
			tgt_g = execute_wggih(cpu, opcode.wggih, imm, tgt_g, src1_g);
			cpu->gpr[instr.wggih.tgt_g] = tgt_g;
			break;
		case WCGIH:
			tgt_g = cpu->gpr[instr.wcgih.tgt_g];
			tgt_c  = MASK(cpu->cond, instr.wcgih.tgt_c);
			tgt_c  = instr.wcgih.neg_tc == 0 ? tgt_c : !tgt_c;
			imm  = instr.wcgih.imm16_lo;
			imm |= instr.wcgih.imm16_hi << 8;
			imm  = (uint32_t) (int16_t) imm;
			tgt_g = execute_wcgih(cpu, opcode.wcgih, imm, tgt_g, tgt_c);
			cpu->gpr[instr.wggih.tgt_g] = tgt_g;
			break;
		case WCC3G:
			src1_g = cpu->gpr[instr.wcc3g.src1_g];
			src2_g  =  cpu->gpr[instr.wcc3g.src2_g];
			tgt_c  = EXTRACT(cpu->cond, 1, instr.wcc3g.tgt_c) != 0;
			tgt_c  = instr.wcc3g.neg_tc == 0 ? tgt_c : !tgt_c;
			src_c  = EXTRACT(cpu->cond, 1, instr.wcc3g.src_c) != 0;
			src_c  = instr.wcc3g.neg_sc == 0 ? src_c : !src_c;
			multi_result_t ret = execute_wcc3g(
				cpu, opcode.wcc3g, instr.wcc3g.funct,
				src1_g, src2_g, tgt_c, src_c, instr.wcc3g.neg_tc);
			cpu->gpr[instr.wcc3g.dst_g] = ret.value;
			cpu->cond &= ~MASK(1, instr.wcc3g.tgt_c);
			cpu->cond |= MASK(ret.flag ? 1 : 0, instr.wcc3g.tgt_c);
			break;
		case INVALID:
			stop_running();
			fprintf(stderr,
				"Execution of undefined opcode: %02xh at %08xh\n",
				instr.opcode, cpu->ip
			);
			break;
	}

	cpu->cond &= 0xFE;
	cpu->gpr[0] = 0;
	cpu->step_count++;
}
