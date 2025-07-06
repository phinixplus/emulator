#include "cpu.h"

#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>

#include "main.h"

/*
I was alerted of attempts to compile the emulator in Windows. For some reason
Windows didn't like `unsigned` in all these bitfields and the packing failed.
Changing these to `unsigned char`, or to `uint8_t` for brevity (forgive me)
seems to fix the issue. My hunch is that windows tried to maintain the 4 byte
alignment of `unsigned` even if they where bitfields. A side-effect of this
is that we can no longer have bitfields bigger than 8 bits, but that makes
it more clear which part goes where, agnostic to endianness of the host.
*/
#pragma pack(push, 1)
typedef union instruction {
	uint32_t opcode_word;
	struct {
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
				uint8_t tgt1_g: 4;
				uint8_t tgt2_g: 4;
				uint8_t funct: 4;
				uint8_t src_g: 4;
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
		cpu->data[i] = cpu->addr[i] = 0;

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
	enum fetch_config {
		REST_DATA = 0,
		TGT1_ADDR = 1 << 0,
		TGT2_ADDR = 1 << 1,
		 SRC_ADDR = 1 << 2,
	} fetch;
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

#define PUT_WGIL( ADDR, TYPE, FETCH) [ADDR] = \
	{.type = WGIL,  .fetch = FETCH, .wgil  = TYPE}
#define PUT_WGGIH(ADDR, TYPE, FETCH) [ADDR] = \
	{.type = WGGIH, .fetch = FETCH, .wggih = TYPE}
#define PUT_WCGIH(ADDR, TYPE, FETCH) [ADDR] = \
	{.type = WCGIH, .fetch = FETCH, .wcgih = TYPE}
#define PUT_WCC3G(ADDR, TYPE, FETCH) [ADDR] = \
	{.type = WCC3G, .fetch = FETCH, .wcc3g = TYPE}

static const opcode_t opcodes[256] = {
	PUT_WGGIH(0x10, INP,    REST_DATA), PUT_WGGIH(0x11, OUT,    REST_DATA),

	PUT_WGGIH(0x14, LBSih,  REST_DATA), PUT_WGGIH(0x15, LBUih,  REST_DATA),
	PUT_WGGIH(0x16, LHSih,  REST_DATA), PUT_WGGIH(0x17, LHUih,  REST_DATA),
	PUT_WGGIH(0x18, LWSih,  REST_DATA), PUT_WGGIH(0x19, LWUih,  REST_DATA),

	PUT_WGGIH(0x1C, SBih,   REST_DATA), PUT_WGGIH(0x1D, SHih,   REST_DATA),
	PUT_WGGIH(0x1E, SWih,   REST_DATA),

	PUT_WGGIH(0x20, ADDih,  REST_DATA), PUT_WGGIH(0x21, ANDih,  REST_DATA),
	PUT_WGGIH(0x22, IORih,  REST_DATA), PUT_WGGIH(0x23, XORih,  REST_DATA),

	PUT_WGGIH(0x28, ALUI,   REST_DATA), PUT_WGGIH(0x29, RLUI,   REST_DATA),

	PUT_WGIL( 0x30, JNLil,  REST_DATA), PUT_WGIL( 0x31,  JMPil, REST_DATA),
	PUT_WCGIH(0x32, CJNLih, REST_DATA), PUT_WCGIH(0x33, CJMPih, REST_DATA),

	PUT_WCC3G(0x40, ADDr,   REST_DATA), PUT_WCC3G(0x41, SUBr,   REST_DATA),
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
	uint32_t value1;
	uint32_t value2;
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
	cpu_t *cpu, enum opcode_wcc3g type,
	uint32_t tgt1_g, uint32_t tgt2_g, uint32_t src_g,
	bool tgt_c, bool src_c, bool neg_tc, uint8_t funct
) {
	(void) cpu, (void) src_c;
	switch(type) {
		bool mix;
		case ADDr:
			tgt1_g = tgt2_g + src_g;
			switch(EXTRACT(funct, 2, 0)) {
				case 0: tgt_c = check_carry(tgt2_g, src_g); break;
				case 1: tgt_c = check_karry(tgt2_g, src_g); break;
				case 2: tgt_c = check_overflow(tgt2_g, src_g); break;
				case 3: break;
			}
			tgt_c = neg_tc ? !tgt_c : tgt_c;
			mix = EXTRACT(funct, 1, 2);
			mix ? (tgt_c &= src_c) : (tgt_c |= src_c);
			break;
		case SUBr:
			tgt1_g = tgt2_g - src_g;
			switch(EXTRACT(funct, 2, 0)) {
				case 0: tgt_c = check_carry(tgt2_g, -src_g); break;
				case 1: tgt_c = check_karry(tgt2_g, src_g); break;
				case 2: tgt_c = check_overflow(tgt2_g, -src_g); break;
				case 3: break;
			}
			tgt_c = neg_tc ? !tgt_c : tgt_c;
			mix = EXTRACT(funct, 1, 2);
			mix ? (tgt_c &= src_c) : (tgt_c |= src_c);
			break;
	}

	cpu->ip += 4;
	return (multi_result_t) {
		.value1 = tgt1_g,
		.value2 = tgt2_g,
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

	uint32_t *tgt1_file = opcode.fetch & TGT1_ADDR ? cpu->addr : cpu->data;
	uint32_t *tgt2_file = opcode.fetch & TGT2_ADDR ? cpu->addr : cpu->data;
	uint32_t *src_file  = opcode.fetch &  SRC_ADDR ? cpu->addr : cpu->data;

	switch(opcode.type) {
		uint32_t imm, tgt1_g, tgt2_g, src_g;
		bool tgt_c, src_c;
		case WGIL:
			tgt1_g = tgt1_file[instr.wgil.tgt_g];
			imm  = instr.wgil.imm20_hi << 4;
			imm  = ((uint32_t) (int8_t) imm) << 12;
			imm |= instr.wgil.imm20_md << 8;
			imm |= instr.wgil.imm20_lo;
			tgt1_g = execute_wgil(cpu, opcode.wgil, imm, tgt1_g);
			tgt1_file[instr.wggih.tgt_g] = tgt1_g;
			break;
		case WGGIH:
			tgt1_g = tgt1_file[instr.wggih.tgt_g];
			src_g  =  src_file[instr.wggih.src_g];
			imm  = instr.wggih.imm16_lo;
			imm |= instr.wggih.imm16_hi << 8;
			imm  = (uint32_t) (int16_t) imm;
			tgt1_g = execute_wggih(cpu, opcode.wggih, imm, tgt1_g, src_g);
			tgt1_file[instr.wggih.tgt_g] = tgt1_g;
			break;
		case WCGIH:
			tgt1_g = tgt1_file[instr.wcgih.tgt_g];
			tgt_c  = MASK(cpu->cond, instr.wcgih.tgt_c);
			tgt_c  = instr.wcgih.neg_tc == 0 ? tgt_c : !tgt_c;
			imm  = instr.wcgih.imm16_lo;
			imm |= instr.wcgih.imm16_hi << 8;
			imm  = (uint32_t) (int16_t) imm;
			tgt1_g = execute_wcgih(cpu, opcode.wcgih, imm, tgt1_g, tgt_c);
			tgt1_file[instr.wggih.tgt_g] = tgt1_g;
			break;
		case WCC3G:
			tgt1_g = tgt1_file[instr.wcc3g.tgt1_g];
			tgt2_g = tgt2_file[instr.wcc3g.tgt2_g];
			src_g  =  src_file[instr.wcc3g.src_g];
			tgt_c  = EXTRACT(cpu->cond, 1, instr.wcc3g.tgt_c) != 0;
			tgt_c  = instr.wcc3g.neg_tc == 0 ? tgt_c : !tgt_c;
			src_c  = EXTRACT(cpu->cond, 1, instr.wcc3g.src_c) != 0;
			src_c  = instr.wcc3g.neg_sc == 0 ? src_c : !src_c;
			multi_result_t ret = execute_wcc3g(
				cpu, opcode.wcc3g, tgt1_g, tgt2_g, src_g,
				tgt_c, src_c, instr.wcc3g.neg_tc, instr.wcc3g.funct);
			tgt1_file[instr.wcc3g.tgt1_g] = ret.value1;
			tgt2_file[instr.wcc3g.tgt2_g] = ret.value2;
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
	cpu->data[0] = 0;
	cpu->step_count++;
}
