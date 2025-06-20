#include "cpu.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

#include "main.h"

typedef unsigned char bits_t;
#pragma pack(push, 1)
typedef union instruction {
	uint32_t instr_word;
	struct {
		uint8_t opcode;
		union {
			struct {
				bits_t tgt_g: 4;
				bits_t imm20_hi: 4;
				bits_t imm20_md: 8;
				bits_t imm20_lo: 8;
			} w_g_il;
			struct {
				bits_t tgt_g: 4;
				bits_t src_g: 4;
				bits_t imm16_hi: 8;
				bits_t imm16_lo: 8;
			} w_gg_ih;
			struct {
				bits_t tgt_g: 4;
				bits_t src1_g: 4;
				bits_t funct: 4;
				bits_t src2_g: 4;
				bits_t tgt_c: 3;
				bits_t neg_tc: 1;
				bits_t src_c: 3;
				bits_t neg_sc: 1;
			} w_cc3g;
		};
	};
} instruction_t;
#pragma pack(pop)

static inline uint32_t get_imm_w_g_il(instruction_t *instr) {
	uint16_t imm = instr->w_g_il.imm20_lo;
	imm |= instr->w_g_il.imm20_md << 8;
	imm |= instr->w_g_il.imm20_hi << 16;
	return imm;
}

static inline uint16_t get_imm_w_gg_ih(instruction_t *instr) {
	uint16_t imm = instr->w_gg_ih.imm16_lo;
	imm |= instr->w_gg_ih.imm16_hi << 8;
	return imm;
}

static inline bool get_cond(cpu_t *cpu, uint8_t pos) {
	uint8_t tmp = EXTRACT(cpu->cond, 1, pos);
	return tmp != 0;
}

static inline void put_cond(cpu_t *cpu, uint8_t pos, bool cond) {
	cpu->cond &= ~MASK(1, pos);
	cpu->cond |= (cond ? 1 : 0) << pos;
}

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

void cpu_execute(cpu_t *cpu) {
	pthread_mutex_lock(&cpu->mutex);
	if(ipm_interrupted(cpu) && !ipm_check_privilege(cpu, false)) {
		cpu->jp = cpu->ip, cpu->ip = cpu->start_addr;
		ipm_set_privilege(cpu, true);
	}
	pthread_mutex_unlock(&cpu->mutex);

	uint32_t inst_word = mem_fetch_word(cpu->mem, cpu->ip);
	instruction_t instr = {inst_word};

	switch(instr.opcode) {
		uint32_t tmp1_g, tmp2_g;
		bool tmp1_c, tmp2_c, tmp3_c;
		case 0x10: // JNLil
			cpu->data[instr.w_g_il.tgt_g] = cpu->ip + 4;
			tmp1_g = get_imm_w_g_il(&instr);
			tmp1_g |= EXTRACT(tmp1_g, 1, 19) != 0 ? MASK(12, 20) : 0;
			cpu->ip += tmp1_g << 1;
			break;
		case 0x20: // INP
			tmp1_g = cpu->data[instr.w_gg_ih.src_g];
			tmp1_g += get_imm_w_gg_ih(&instr);
			tmp2_g = io_read(cpu->io, tmp1_g & 0xFFFF);
			cpu->data[instr.w_gg_ih.tgt_g] = tmp2_g;

			cpu->ip += 4;
			break;
		case 0x21: // OUT
			tmp1_g = cpu->data[instr.w_gg_ih.src_g];
			tmp1_g += get_imm_w_gg_ih(&instr);
			tmp2_g = cpu->data[instr.w_gg_ih.tgt_g];
			io_write(cpu->io, tmp1_g & 0xFFFF, tmp2_g);

			cpu->ip += 4;
			break;
		case 0x22: // ADDih
			tmp1_g = cpu->data[instr.w_gg_ih.src_g];
			tmp1_g += (int16_t) get_imm_w_gg_ih(&instr);
			cpu->data[instr.w_gg_ih.tgt_g] = tmp1_g;

			cpu->ip += 4;
			break;
		case 0x40: // ADDr
			tmp1_g = cpu->data[instr.w_cc3g.src1_g];
			tmp2_g = tmp1_g + cpu->data[instr.w_cc3g.src2_g];
			cpu->data[instr.w_cc3g.tgt_g] = tmp2_g;

			tmp1_c = get_cond(cpu, instr.w_cc3g.src_c) ^ instr.w_cc3g.neg_sc;
			tmp2_c = (tmp2_g < tmp1_g) ^ instr.w_cc3g.neg_tc;
			tmp3_c = EXTRACT(instr.w_cc3g.funct, 1, 0);
			tmp3_c = tmp3_c ? tmp1_c & tmp2_c : tmp1_c | tmp2_c;
			put_cond(cpu, instr.w_cc3g.tgt_c, tmp3_c);

			cpu->ip += 4;
			break;
		case 0x41: // SUBr
			tmp1_g = cpu->data[instr.w_cc3g.src1_g];
			tmp2_g = tmp1_g - cpu->data[instr.w_cc3g.src2_g];
			cpu->data[instr.w_cc3g.tgt_g] = tmp2_g;

			tmp1_c = get_cond(cpu, instr.w_cc3g.src_c) ^ instr.w_cc3g.neg_sc;
			tmp2_c = (tmp2_g < tmp1_g) ^ instr.w_cc3g.neg_tc;
			tmp3_c = EXTRACT(instr.w_cc3g.funct, 1, 0);
			tmp3_c = tmp3_c ? tmp1_c & tmp2_c : tmp1_c | tmp2_c;
			put_cond(cpu, instr.w_cc3g.tgt_c, tmp3_c);

			cpu->ip += 4;
			break;
		default:
			stop_running();
			fprintf(stderr,
				"Execution of undefined opcode: %02xh at %08xh\n",
				instr.opcode, cpu->ip
			);
	}

	cpu->cond &= 0xFE;
	cpu->data[0] = 0;
	cpu->step_count++;
}
