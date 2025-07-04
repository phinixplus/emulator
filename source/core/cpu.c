#include "cpu.h"

#include <assert.h>
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
	uint32_t instr_word;
	struct {
		uint8_t opcode;
		union {
			struct {
				uint8_t tgt_g: 4;
				uint8_t imm20_hi: 4;
				uint8_t imm20_md: 8;
				uint8_t imm20_lo: 8;
			} w_g_il;
			struct {
				uint8_t tgt_g: 4;
				uint8_t src_g: 4;
				uint8_t imm16_hi: 8;
				uint8_t imm16_lo: 8;
			} w_gg_ih;
			struct {
				uint8_t tgt_g: 4;
				uint8_t tgt_c: 3;
				uint8_t neg_tc: 1;
				uint8_t imm16_hi: 8;
				uint8_t imm16_lo: 8;
			} w_cg_ih;
			struct {
				uint8_t tgt_g: 4;
				uint8_t src1_g: 4;
				uint8_t funct: 4;
				uint8_t src2_g: 4;
				uint8_t tgt_c: 3;
				uint8_t neg_tc: 1;
				uint8_t src_c: 3;
				uint8_t neg_sc: 1;
			} w_cc3g;
		};
	};
} instruction_t;
#pragma pack(pop)

typedef struct opcode {
	enum instr_type {
		INVALID = 0, W_G_IL,
		W_GG_IH, W_CG_IH, W_CC3G
	} type;
	union {
		enum instr_w_g_il {
			JNLil, JMPil
		} w_g_il;
		enum instr_w_gg_ih {
			INP, OUT,
			LBSih, LBUih, SBih,
			LHSih, LHUih, SHih,
			LWSih, LWUih, SWih,
			ADDih, ANDih, IORih, XORih,
			ALUI, RLUI,
		} w_gg_ih;
		enum instr_w_cg_ih {
			CJNLih, CJMPih
		} w_cg_ih;
		enum instr_w_cc3g {
			ADDr, SUBr
		} w_cc3g;
	};
} opcode_t;

#define PUT_WGIL( ADDR, TYPE) [ADDR] = {.type = W_G_IL,  .w_g_il  = TYPE}
#define PUT_WGGIH(ADDR, TYPE) [ADDR] = {.type = W_GG_IH, .w_gg_ih = TYPE}
#define PUT_WCGIH(ADDR, TYPE) [ADDR] = {.type = W_CG_IH, .w_cg_ih = TYPE}
#define PUT_WCC3G(ADDR, TYPE) [ADDR] = {.type = W_CC3G,  .w_cc3g  = TYPE}
static const opcode_t opcodes[256] = {
	PUT_WGGIH(0x10, INP),   PUT_WGGIH(0x11, OUT),

	PUT_WGGIH(0x14, LBSih),  PUT_WGGIH(0x15, LBUih),
	PUT_WGGIH(0x16, LHSih),  PUT_WGGIH(0x17, LHUih),
	PUT_WGGIH(0x18, LWSih),  PUT_WGGIH(0x19, LWUih),

	PUT_WGGIH(0x1C, SBih),   PUT_WGGIH(0x1D, SHih), PUT_WGGIH(0x1E, SWih),

	PUT_WGGIH(0x20, ADDih),  PUT_WGGIH(0x21, ANDih),
	PUT_WGGIH(0x22, IORih),  PUT_WGGIH(0x23, XORih),

	PUT_WGGIH(0x28, ALUI),   PUT_WGGIH(0x29, RLUI),

	PUT_WGIL( 0x30, JNLil),  PUT_WGIL( 0x31,  JMPil),
	PUT_WCGIH(0x32, CJNLih), PUT_WCGIH(0x33, CJMPih),

	PUT_WCC3G(0x40, ADDr),   PUT_WCC3G(0x41, SUBr),
};

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

static uint32_t execute_w_g_il(
	cpu_t *cpu, enum instr_w_g_il type,
	uint32_t imm, uint32_t tgt_g
) {
	uint32_t ret = tgt_g;
	switch(type) {
		case JNLil:
			break;
		case JMPil:
			break;
	}
	return ret;
}

static uint32_t execute_w_gg_ih(
	cpu_t *cpu, enum instr_w_gg_ih type,
	uint32_t imm, uint32_t tgt_g, uint32_t src_g
) {
	uint32_t ret = tgt_g;
	switch(type) {
		case INP: ret = io_read(cpu->io, src_g + imm); break;
		case OUT: io_write(cpu->io, src_g + imm, tgt_g); break;
		case LBSih:
		case LBUih:
		case SBih:
		case LHSih:
		case LHUih:
		case SHih:
		case LWSih:
		case LWUih:
		case SWih:
			break;
		case ADDih: ret = src_g + imm; break;
		case ANDih: ret = src_g & imm; break;
		case IORih: ret = src_g | imm; break;
		case XORih: ret = src_g ^ imm; break;
		case ALUI:
		case RLUI:
			break;
	}

	cpu->ip += 4;
	return ret;
}

static uint32_t execute_w_cg_ih(
	cpu_t *cpu, enum instr_w_cg_ih type,
	uint32_t imm, uint32_t tgt_g, bool tgt_c
) {
	switch(type) {
		case CJNLih:
			break;
		case CJMPih:
			break;
	}
}

static struct w_cc3g_ret {uint32_t g; bool c;} execute_w_cc3g(
	cpu_t *cpu, enum instr_w_cc3g type,
	uint32_t tgt_g, uint32_t src1_g, uint32_t src2_g,
	bool tgt_c, bool src_c
) {
	uint32_t ret_g = tgt_g;
	bool ret_c = tgt_c;
	switch(type) {
		case ADDr:
			break;
		case SUBr:
			break;
	}

	return (struct w_cc3g_ret){.g = ret_g, .c = ret_c};
}

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
		case W_G_IL:
			tgt_g = cpu->data[instr.w_g_il.tgt_g];
			imm  = instr.w_g_il.imm20_hi << 4;
			imm  = ((uint32_t) (int8_t) imm) << 12;
			imm |= instr.w_g_il.imm20_md << 8;
			imm |= instr.w_g_il.imm20_lo;
			tgt_g = execute_w_g_il(cpu, opcode.w_g_il, imm, tgt_g);
			cpu->data[instr.w_gg_ih.tgt_g] = tgt_g;
			break;
		case W_GG_IH:
			tgt_g  = cpu->data[instr.w_gg_ih.tgt_g];
			src1_g = cpu->data[instr.w_gg_ih.src_g];
			imm  = instr.w_gg_ih.imm16_lo;
			imm |= instr.w_gg_ih.imm16_hi << 8;
			imm  = (uint32_t) (int16_t) imm;
			tgt_g = execute_w_gg_ih(cpu, opcode.w_gg_ih, imm, tgt_g, src1_g);
			cpu->data[instr.w_gg_ih.tgt_g] = tgt_g;
			break;
		case W_CG_IH:
			break;
		case W_CC3G:
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
