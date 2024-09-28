#subruledef datareg {
	x0 => 0x0`4
	x1 => 0x1`4
	x2 => 0x2`4
	x3 => 0x3`4
	x4 => 0x4`4
	x5 => 0x5`4
	x6 => 0x6`4
	x7 => 0x7`4
	x8 => 0x8`4
	x9 => 0x9`4
	xA => 0xA`4
	xB => 0xB`4
	xC => 0xC`4
	xD => 0xD`4
	xE => 0xE`4
	xF => 0xF`4

	zr => 0x0`4
	at => 0x1`4
	rp => 0x2`4
	t0 => 0x3`4
	t1 => 0x4`4
	t2 => 0x5`4
	a0 => 0x6`4
	a1 => 0x7`4
	a2 => 0x8`4
	s0 => 0x9`4
	s1 => 0xA`4
	s2 => 0xB`4
	s3 => 0xC`4
	s4 => 0xD`4
	s5 => 0xE`4
	fp => 0xF`4
}

#subruledef addrreg {
	y0 => 0x0`4
	y1 => 0x1`4
	y2 => 0x2`4
	y3 => 0x3`4
	y4 => 0x4`4
	y5 => 0x5`4
	y6 => 0x6`4
	y7 => 0x7`4
	y8 => 0x8`4
	y9 => 0x9`4
	yA => 0xA`4
	yB => 0xB`4
	yC => 0xC`4
	yD => 0xD`4
	yE => 0xE`4
	yF => 0xF`4

	a3 => 0x0`4
	a4 => 0x1`4
	a5 => 0x2`4
	t3 => 0x3`4
	t4 => 0x4`4
	t5 => 0x5`4
	t6 => 0x6`4
	t7 => 0x7`4
	s6 => 0x8`4
	s7 => 0x9`4
	gp => 0xA`4
	sp => 0xB`4
	k0 => 0xC`4
	k1 => 0xD`4
	k2 => 0xE`4
	kp => 0xF`4
}

#subruledef condreg {
	c0 => 0x0`3
	c1 => 0x1`3
	c2 => 0x2`3
	c3 => 0x3`3
	c4 => 0x4`3
	c5 => 0x5`3
	c6 => 0x6`3
	c7 => 0x7`3
}

; ---------------------------------------------------------------------------- ;

;                 |         
; _15___12_11____8|7______0_
; | funct | tgt.g | opcode |
; |_______|_______|________|
;                 |         

#ruledef native_hg {
	hlt => 0x00`8 @ 0x0`4 @ 0x0`4
	rsm => 0x00`8 @ 0x1`4 @ 0x0`4
	mov jp {src: datareg} => 0x00`8 @ 0x2`4 @ {src}
	mov {dst: datareg} jp => 0x00`8 @ 0x3`4 @ {dst}
	pad => (pc & 2 != 0 ? 0x00`8 @ 0xF`4 @ 0x0`4 : 0`0)
}

;                 |         
; _15___12_11____8|7______0_
; | src.g | tgt.g | opcode |
; |_______|_______|________|
;                 |         

#ruledef native_hgg {
	mov {dst: datareg} {src: datareg} => 0x01`8 @ {src} @ {dst}
	mov {dst: datareg} {src: addrreg} => 0x02`8 @ {src} @ {dst}
	mov {dst: addrreg} {src: datareg} => 0x03`8 @ {src} @ {dst}
	mov {dst: addrreg} {src: addrreg} => 0x04`8 @ {src} @ {dst}
	add {dst: datareg} {src: datareg} => 0x05`8 @ {src} @ {dst}
	add {dst: addrreg} {src: addrreg} => 0x06`8 @ {src} @ {dst}
	sub {dst: datareg} {src: datareg} => 0x07`8 @ {src} @ {dst}
	sub {dst: addrreg} {src: addrreg} => 0x08`8 @ {src} @ {dst}

	and {dst: datareg} {src: datareg} => 0x09`8 @ {src} @ {dst}
	nnd {dst: datareg} {src: datareg} => 0x0A`8 @ {src} @ {dst}
	ior {dst: datareg} {src: datareg} => 0x0B`8 @ {src} @ {dst}
	nor {dst: datareg} {src: datareg} => 0x0C`8 @ {src} @ {dst}
	xor {dst: datareg} {src: datareg} => 0x0D`8 @ {src} @ {dst}

	sl  {dst: datareg} {src: datareg} => 0x0E`8 @ {src} @ {dst}
	sru {dst: datareg} {src: datareg} => 0x0F`8 @ {src} @ {dst}
	srs {dst: datareg} {src: datareg} => 0x10`8 @ {src} @ {dst}
	bl  {dst: datareg} {src: datareg} => 0x11`8 @ {src} @ {dst}
	bru {dst: datareg} {src: datareg} => 0x12`8 @ {src} @ {dst}
	brs {dst: datareg} {src: datareg} => 0x13`8 @ {src} @ {dst}

	lbu {tgt: datareg} {addr: datareg} => 0x14`8 @ {addr} @ {tgt}
	lbu {tgt: datareg} {addr: addrreg} => 0x15`8 @ {addr} @ {tgt}
	lbs {tgt: datareg} {addr: datareg} => 0x16`8 @ {addr} @ {tgt}
	lbs {tgt: datareg} {addr: addrreg} => 0x17`8 @ {addr} @ {tgt}
	sb  {tgt: datareg} {addr: datareg} => 0x18`8 @ {addr} @ {tgt}
	sb  {tgt: datareg} {addr: addrreg} => 0x19`8 @ {addr} @ {tgt}
	lhu {tgt: datareg} {addr: datareg} => 0x1A`8 @ {addr} @ {tgt}
	lhu {tgt: datareg} {addr: addrreg} => 0x1B`8 @ {addr} @ {tgt}
	lhs {tgt: datareg} {addr: datareg} => 0x1C`8 @ {addr} @ {tgt}
	lhs {tgt: datareg} {addr: addrreg} => 0x1D`8 @ {addr} @ {tgt}
	sh  {tgt: datareg} {addr: datareg} => 0x1E`8 @ {addr} @ {tgt}
	sh  {tgt: datareg} {addr: addrreg} => 0x1F`8 @ {addr} @ {tgt}
	lw  {tgt: datareg} {addr: datareg} => 0x20`8 @ {addr} @ {tgt}
	lw  {tgt: datareg} {addr: addrreg} => 0x21`8 @ {addr} @ {tgt}
	sw  {tgt: datareg} {addr: datareg} => 0x22`8 @ {addr} @ {tgt}
	sw  {tgt: datareg} {addr: addrreg} => 0x23`8 @ {addr} @ {tgt}

	jnl {dst: datareg} {src: datareg} => 0x24`8 @ {src} @ {dst}
	jnl {dst: datareg} {src: addrreg} => 0x25`8 @ {src} @ {dst}
	jnl {dst: addrreg} {src: datareg} => 0x26`8 @ {src} @ {dst}
	jnl {dst: addrreg} {src: addrreg} => 0x27`8 @ {src} @ {dst}
}

;                       |         
; _15____14___12_11____8|7______0_
; | neg | tgt.c | tgt.g | opcode |
; |_____|_______|_______|________|
;                       |         

#ruledef native_hcg {
	tst zer {cnd: condreg} {gpr: datareg} => 0x28`8 @ 0`1 @ {cnd} @ {gpr}
	tst nzr {cnd: condreg} {gpr: datareg} => 0x28`8 @ 1`1 @ {cnd} @ {gpr}
	tst zer {cnd: condreg} {gpr: addrreg} => 0x29`8 @ 0`1 @ {cnd} @ {gpr}
	tst nzr {cnd: condreg} {gpr: addrreg} => 0x29`8 @ 1`1 @ {cnd} @ {gpr}
	tst neg {cnd: condreg} {gpr: datareg} => 0x2A`8 @ 0`1 @ {cnd} @ {gpr}
	tst pos {cnd: condreg} {gpr: datareg} => 0x2A`8 @ 1`1 @ {cnd} @ {gpr}
	tst neg {cnd: condreg} {gpr: addrreg} => 0x2B`8 @ 0`1 @ {cnd} @ {gpr}
	tst pos {cnd: condreg} {gpr: addrreg} => 0x2B`8 @ 1`1 @ {cnd} @ {gpr}
	tst odd {cnd: condreg} {gpr: datareg} => 0x2C`8 @ 0`1 @ {cnd} @ {gpr}
	tst evn {cnd: condreg} {gpr: datareg} => 0x2C`8 @ 1`1 @ {cnd} @ {gpr}
	tst odd {cnd: condreg} {gpr: addrreg} => 0x2D`8 @ 0`1 @ {cnd} @ {gpr}
	tst evn {cnd: condreg} {gpr: addrreg} => 0x2D`8 @ 1`1 @ {cnd} @ {gpr}

	jmp    {gpr: datareg} if  {cnd: condreg} => 0x2E`8 @ 0`1 @ {cnd} @ {gpr}
	jmp    {gpr: datareg} if !{cnd: condreg} => 0x2E`8 @ 1`1 @ {cnd} @ {gpr}
	jmp    {gpr: addrreg} if  {cnd: condreg} => 0x2F`8 @ 0`1 @ {cnd} @ {gpr}
	jmp    {gpr: addrreg} if !{cnd: condreg} => 0x2F`8 @ 1`1 @ {cnd} @ {gpr}
	jmp ip {gpr: datareg} if  {cnd: condreg} => 0x30`8 @ 0`1 @ {cnd} @ {gpr}
	jmp ip {gpr: datareg} if !{cnd: condreg} => 0x30`8 @ 1`1 @ {cnd} @ {gpr}
	jmp ip {gpr: addrreg} if  {cnd: condreg} => 0x31`8 @ 0`1 @ {cnd} @ {gpr}
	jmp ip {gpr: addrreg} if !{cnd: condreg} => 0x31`8 @ 1`1 @ {cnd} @ {gpr}
}

;               |         
; _15_12_11____8|7______0_
; | imm | tgt.g | opcode |
; |_____|_______|________|
;               |         

#ruledef native_hgi {
	add {gpr: datareg} {imm: u5} => {
		assert(imm >= 1 && imm <= 16)
		0x32`8 @ (imm-1)`4 @ {gpr}
	}
	add {gpr: addrreg} {imm: u5} => {
		assert(imm >= 1 && imm <= 16)
		0x33`8 @ (imm-1)`4 @ {gpr}
	}
	sub {gpr: datareg} {imm: u5} => {
		assert(imm >= 1 && imm <= 16)
		0x34`8 @ (imm-1)`4 @ {gpr}
	}
	sub {gpr: addrreg} {imm: u5} => {
		assert(imm >= 1 && imm <= 16)
		0x35`8 @ (imm-1)`4 @ {gpr}
	}

	bl  {gpr: datareg} {imm: u5} => {
		assert(imm >= 1 && imm <= 16)
		0x36`8 @ (imm-1)`4 @ {gpr}
	}
	bru {gpr: datareg} {imm: u5} => {
		assert(imm >= 1 && imm <= 16)
		0x37`8 @ (imm-1)`4 @ {gpr}
	}
	brs {gpr: datareg} {imm: u5} => {
		assert(imm >= 2 && imm <= 17)
		0x38`8 @ (imm-2)`4 @ {gpr}
	}
}

; ---------------------------------------------------------------------------- ;

;       |               |         
; _31_16|15___12_11____8|7______0_
; | imm | src.g | tgt.g | opcode |
; |_____|_______|_______|________|
;       |               |         

#ruledef native_wggi {
	add {dst: datareg} {src: datareg}  {imm: s16} => asm {pad} @ 0x39 @ {src} @ {dst} @ le(imm)
	add {dst: addrreg} {src: addrreg}  {imm: s16} => asm {pad} @ 0x3A @ {src} @ {dst} @ le(imm)
	add {dst: datareg} {src: datareg} ^{imm: s16} => asm {pad} @ 0x3B @ {src} @ {dst} @ le(imm)
	add {dst: addrreg} {src: addrreg} ^{imm: s16} => asm {pad} @ 0x3C @ {src} @ {dst} @ le(imm)

	and {dst: datareg} {src: datareg}  {imm: u16} => asm {pad} @ 0x3D @ {src} @ {dst} @ le(imm)
	and {dst: datareg} {src: datareg} ^{imm: u16} => asm {pad} @ 0x3E @ {src} @ {dst} @ le(imm)
	ior {dst: datareg} {src: datareg}  {imm: u16} => asm {pad} @ 0x3F @ {src} @ {dst} @ le(imm)
	ior {dst: datareg} {src: datareg} ^{imm: u16} => asm {pad} @ 0x40 @ {src} @ {dst} @ le(imm)
	xor {dst: datareg} {src: datareg}  {imm: u16} => asm {pad} @ 0x41 @ {src} @ {dst} @ le(imm)
	xor {dst: datareg} {src: datareg} ^{imm: u16} => asm {pad} @ 0x42 @ {src} @ {dst} @ le(imm)

	in  {tgt: datareg} {addr: datareg}  {imm: s16} => asm {pad} @ 0x43 @ {addr} @ {tgt} @ le(imm)
	out {tgt: datareg} {addr: datareg}  {imm: s16} => asm {pad} @ 0x44 @ {addr} @ {tgt} @ le(imm)
}

;       |                     |         
; _31_16|15____14___12_11____8|7______0_
; | imm | neg | tgt.c | tgt.g | opcode |
; |_____|_____|_______|_______|________|
;       |                     |         

#ruledef native_wcgi {
	jmp    {gpr: datareg} {imm: s16} if  {cnd: condreg} => asm {pad} @ 0x45`8 @ 0`1 @ {cnd} @ {gpr} @ le(imm)
	jmp    {gpr: datareg} {imm: s16} if !{cnd: condreg} => asm {pad} @ 0x45`8 @ 1`1 @ {cnd} @ {gpr} @ le(imm)
	jmp    {gpr: addrreg} {imm: s16} if  {cnd: condreg} => asm {pad} @ 0x46`8 @ 0`1 @ {cnd} @ {gpr} @ le(imm)
	jmp    {gpr: addrreg} {imm: s16} if !{cnd: condreg} => asm {pad} @ 0x46`8 @ 1`1 @ {cnd} @ {gpr} @ le(imm)
	jmp ip {gpr: datareg} {imm: s16} if  {cnd: condreg} => asm {pad} @ 0x47`8 @ 0`1 @ {cnd} @ {gpr} @ le(imm)
	jmp ip {gpr: datareg} {imm: s16} if !{cnd: condreg} => asm {pad} @ 0x47`8 @ 1`1 @ {cnd} @ {gpr} @ le(imm)
	jmp ip {gpr: addrreg} {imm: s16} if  {cnd: condreg} => asm {pad} @ 0x48`8 @ 0`1 @ {cnd} @ {gpr} @ le(imm)
	jmp ip {gpr: addrreg} {imm: s16} if !{cnd: condreg} => asm {pad} @ 0x48`8 @ 1`1 @ {cnd} @ {gpr} @ le(imm)
}

; ---------------------------------------------------------------------------- ;

#ruledef pseudo {
	jmp    {gpr: datareg} => asm { jmp    {gpr} if !c0 }
	jmp    {gpr: addrreg} => asm { jmp    {gpr} if !c0 }
	jmp ip {gpr: datareg} => asm { jmp ip {gpr} if !c0 }
	jmp ip {gpr: addrreg} => asm { jmp ip {gpr} if !c0 }

	add {tgt: datareg}  {imm: s16} => asm { add {tgt} {tgt}  {imm} }
	add {tgt: addrreg}  {imm: s16} => asm { add {tgt} {tgt}  {imm} }
	add {tgt: datareg} ^{imm: s16} => asm { add {tgt} {tgt} ^{imm} }
	add {tgt: addrreg} ^{imm: s16} => asm { add {tgt} {tgt} ^{imm} }

	and {tgt: datareg}  {imm: u16} => asm { and {tgt} {tgt}  {imm} }
	and {tgt: datareg} ^{imm: u16} => asm { and {tgt} {tgt} ^{imm} }
	ior {tgt: datareg}  {imm: u16} => asm { ior {tgt} {tgt}  {imm} }
	ior {tgt: datareg} ^{imm: u16} => asm { ior {tgt} {tgt} ^{imm} }
	xor {tgt: datareg}  {imm: u16} => asm { xor {tgt} {tgt}  {imm} }
	xor {tgt: datareg} ^{imm: u16} => asm { xor {tgt} {tgt} ^{imm} }

	in  {tgt: datareg} {imm: s16} => asm { in  {tgt} x0 {imm} }
	out {tgt: datareg} {imm: s16} => asm { out {tgt} x0 {imm} }

	jmp    {gpr: datareg} {imm: s16} => asm { jmp    {gpr} {imm} if !c0 }
	jmp    {gpr: addrreg} {imm: s16} => asm { jmp    {gpr} {imm} if !c0 }
	jmp ip {gpr: datareg} {imm: s16} => asm { jmp ip {gpr} {imm} if !c0 }
	jmp ip {gpr: addrreg} {imm: s16} => asm { jmp ip {gpr} {imm} if !c0 }
	jmp ip {label: u32} if {cnd: condreg} => {
		offset = label - pc - (pc & 2 != 0 ? 2 : 0)
		assert(offset[31:15] == 0 || offset[31:15] == -1`17)
		asm { jmp ip zr {offset} if {cnd} }
	}
	jmp ip {label: u32} if !{cnd: condreg} => {
		offset = label - pc - (pc & 2 != 0 ? 2 : 0)
		assert(offset[31:15] == 0 || offset[31:15] == -1`17)
		asm { jmp ip zr {offset} if !{cnd} }
	}
	jmp ip {label: u32} => {
		offset = label - pc - (pc & 2 != 0 ? 2 : 0)
		assert(offset[31:15] == 0 || offset[31:15] == -1`17)
		asm { jmp ip zr {offset} if !c0 }
	}
}

#ruledef macro {
	li  {dst: datareg} {imm: i32} => asm {
		ior {dst} zr {imm}[15:0]
		ior {dst} ^{imm}[31:16]
	}

	jmp {label: u32} if {cnd: condreg} => asm {
		li at {label}
		jmp at if {cnd}
	}
	jmp {label: u32} if !{cnd: condreg} => asm {
		li at {label}
		jmp at if !{cnd}
	}
	jmp {label: u32} => asm {
		li at {label}
		jmp at if !c0
	}
}
