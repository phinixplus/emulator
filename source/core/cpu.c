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
		uint32_t tmp_g1, tmp_g2;
		case 0x20: // INP
			tmp_g1 = cpu->data[instr.w_gg_ih.src_g];
			tmp_g1 += get_imm_w_gg_ih(&instr);
			tmp_g2 = io_read(cpu->io, tmp_g1 & 0xFFFF);
			cpu->data[instr.w_gg_ih.tgt_g] = tmp_g2;
			break;
		case 0x21: // OUT
			tmp_g1 = cpu->data[instr.w_gg_ih.src_g];
			tmp_g1 += get_imm_w_gg_ih(&instr);
			tmp_g2 = cpu->data[instr.w_gg_ih.tgt_g];
			io_write(cpu->io, tmp_g1 & 0xFFFF, tmp_g2);
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
