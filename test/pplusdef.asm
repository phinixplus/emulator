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

#ruledef half_gpr_gpr {
	mov {dst: datareg} {src: datareg} => 0x01`8 @ {dst} @ {src}
	mov {dst: datareg} {src: addrreg} => 0x02`8 @ {dst} @ {src}
	mov {dst: addrreg} {src: datareg} => 0x03`8 @ {dst} @ {src}
	mov {dst: addrreg} {src: addrreg} => 0x04`8 @ {dst} @ {src}
	add {dst: datareg} {src: datareg} => 0x05`8 @ {dst} @ {src}
	add {dst: addrreg} {src: addrreg} => 0x06`8 @ {dst} @ {src}
	sub {dst: datareg} {src: datareg} => 0x07`8 @ {dst} @ {src}
	sub {dst: addrreg} {src: addrreg} => 0x08`8 @ {dst} @ {src}

	and {dst: datareg} {src: datareg} => 0x09`8 @ {dst} @ {src}
	nnd {dst: datareg} {src: datareg} => 0x0A`8 @ {dst} @ {src}
	ior {dst: datareg} {src: datareg} => 0x0B`8 @ {dst} @ {src}
	nor {dst: datareg} {src: datareg} => 0x0C`8 @ {dst} @ {src}
	xor {dst: datareg} {src: datareg} => 0x0D`8 @ {dst} @ {src}

	sl  {dst: datareg} {src: datareg} => 0x0E`8 @ {dst} @ {src}
	sru {dst: datareg} {src: datareg} => 0x0F`8 @ {dst} @ {src}
	srs {dst: datareg} {src: datareg} => 0x10`8 @ {dst} @ {src}
	bl  {dst: datareg} {src: datareg} => 0x11`8 @ {dst} @ {src}
	bru {dst: datareg} {src: datareg} => 0x12`8 @ {dst} @ {src}
	brs {dst: datareg} {src: datareg} => 0x13`8 @ {dst} @ {src}

	lbu {tgt: datareg} {addr: datareg} => 0x14`8 @ {tgt} @ {addr}
	lbu {tgt: datareg} {addr: addrreg} => 0x15`8 @ {tgt} @ {addr}
	lbs {tgt: datareg} {addr: datareg} => 0x16`8 @ {tgt} @ {addr}
	lbs {tgt: datareg} {addr: addrreg} => 0x17`8 @ {tgt} @ {addr}
	sb  {tgt: datareg} {addr: datareg} => 0x18`8 @ {tgt} @ {addr}
	sb  {tgt: datareg} {addr: addrreg} => 0x19`8 @ {tgt} @ {addr}
	lhu {tgt: datareg} {addr: datareg} => 0x1A`8 @ {tgt} @ {addr}
	lhu {tgt: datareg} {addr: addrreg} => 0x1B`8 @ {tgt} @ {addr}
	lhs {tgt: datareg} {addr: datareg} => 0x1C`8 @ {tgt} @ {addr}
	lhs {tgt: datareg} {addr: addrreg} => 0x1D`8 @ {tgt} @ {addr}
	sh  {tgt: datareg} {addr: datareg} => 0x1E`8 @ {tgt} @ {addr}
	sh  {tgt: datareg} {addr: addrreg} => 0x1F`8 @ {tgt} @ {addr}
	lw  {tgt: datareg} {addr: datareg} => 0x20`8 @ {tgt} @ {addr}
	lw  {tgt: datareg} {addr: addrreg} => 0x21`8 @ {tgt} @ {addr}
	sw  {tgt: datareg} {addr: datareg} => 0x22`8 @ {tgt} @ {addr}
	sw  {tgt: datareg} {addr: addrreg} => 0x23`8 @ {tgt} @ {addr}

	jnl {dst: datareg} {src: datareg} => 0x24`8 @ {dst} @ {src}
	jnl {dst: datareg} {src: addrreg} => 0x25`8 @ {dst} @ {src}
	jnl {dst: addrreg} {src: datareg} => 0x26`8 @ {dst} @ {src}
	jnl {dst: addrreg} {src: addrreg} => 0x27`8 @ {dst} @ {src}
}

#ruledef half_gpr_cnd {
	tst zer {cnd: condreg} {gpr: datareg} => 0x28`8 @ {gpr} @ 0`1 @ {cnd}
	tst nzr {cnd: condreg} {gpr: datareg} => 0x28`8 @ {gpr} @ 1`1 @ {cnd}
	tst zer {cnd: condreg} {gpr: addrreg} => 0x29`8 @ {gpr} @ 0`1 @ {cnd}
	tst nzr {cnd: condreg} {gpr: addrreg} => 0x29`8 @ {gpr} @ 1`1 @ {cnd}
	tst neg {cnd: condreg} {gpr: datareg} => 0x2A`8 @ {gpr} @ 0`1 @ {cnd}
	tst pos {cnd: condreg} {gpr: datareg} => 0x2A`8 @ {gpr} @ 1`1 @ {cnd}
	tst neg {cnd: condreg} {gpr: addrreg} => 0x2B`8 @ {gpr} @ 0`1 @ {cnd}
	tst pos {cnd: condreg} {gpr: addrreg} => 0x2B`8 @ {gpr} @ 1`1 @ {cnd}
	tst odd {cnd: condreg} {gpr: datareg} => 0x2C`8 @ {gpr} @ 0`1 @ {cnd}
	tst evn {cnd: condreg} {gpr: datareg} => 0x2C`8 @ {gpr} @ 1`1 @ {cnd}
	tst odd {cnd: condreg} {gpr: addrreg} => 0x2D`8 @ {gpr} @ 0`1 @ {cnd}
	tst evn {cnd: condreg} {gpr: addrreg} => 0x2D`8 @ {gpr} @ 1`1 @ {cnd}

	jmp {gpr: datareg} if {cnd: condreg}     => 0x2E`8 @ {gpr} @ 0`1 @ {cnd}
	jmp {gpr: datareg} if !{cnd: condreg}    => 0x2E`8 @ {gpr} @ 1`1 @ {cnd}
	jmp {gpr: addrreg} if {cnd: condreg}     => 0x2F`8 @ {gpr} @ 0`1 @ {cnd}
	jmp {gpr: addrreg} if !{cnd: condreg}    => 0x2F`8 @ {gpr} @ 1`1 @ {cnd}
	jmp ip {gpr: datareg} if {cnd: condreg}  => 0x30`8 @ {gpr} @ 0`1 @ {cnd}
	jmp ip {gpr: datareg} if !{cnd: condreg} => 0x30`8 @ {gpr} @ 1`1 @ {cnd}
	jmp ip {gpr: addrreg} if {cnd: condreg}  => 0x31`8 @ {gpr} @ 0`1 @ {cnd}
	jmp ip {gpr: addrreg} if !{cnd: condreg} => 0x31`8 @ {gpr} @ 1`1 @ {cnd}
}

#ruledef half_gpr_imm {
	add {gpr: datareg} {imm: u5} => {
		assert(imm >= 1 && imm <= 16)
		0x32`8 @ {gpr} @ (imm-1)`4
	}
	add {gpr: addrreg} {imm: u5} => {
		assert(imm >= 1 && imm <= 16)
		0x33`8 @ {gpr} @ (imm-1)`4
	}
	sub {gpr: datareg} {imm: u5} => {
		assert(imm >= 1 && imm <= 16)
		0x34`8 @ {gpr} @ (imm-1)`4
	}
	sub {gpr: addrreg} {imm: u5} => {
		assert(imm >= 1 && imm <= 16)
		0x35`8 @ {gpr} @ (imm-1)`4
	}

	bl  {gpr: datareg} {imm: u5} => {
		assert(imm >= 1 && imm <= 16)
		0x36`8 @ {gpr} @ (imm-1)`4
	}
	bru {gpr: datareg} {imm: u5} => {
		assert(imm >= 1 && imm <= 16)
		0x37`8 @ {gpr} @ (imm-1)`4
	}
	brs {gpr: datareg} {imm: u5} => {
		assert(imm >= 2 && imm <= 17)
		0x38`8 @ {gpr} @ (imm-2)`4
	}
}

#ruledef word_gpr_gpr_imm {
	add {dst: datareg} {src: datareg} {imm: s16}  =>
		(pc & 2 != 0 ? asm {pad} : 0`0) @ 0x39 @ {dst} @ {src} @ le(imm)
	add {dst: addrreg} {src: addrreg} {imm: s16}  =>
		(pc & 2 != 0 ? asm {pad} : 0`0) @ 0x3A @ {dst} @ {src} @ le(imm)
	add {dst: datareg} {src: datareg} ^{imm: s16} =>
		(pc & 2 != 0 ? asm {pad} : 0`0) @ 0x3B @ {dst} @ {src} @ le(imm)
	add {dst: addrreg} {src: addrreg} ^{imm: s16} =>
		(pc & 2 != 0 ? asm {pad} : 0`0) @ 0x3C @ {dst} @ {src} @ le(imm)

	and {dst: datareg} {src: datareg} {imm: u16}  =>
		(pc & 2 != 0 ? asm {pad} : 0`0) @ 0x3D @ {dst} @ {src} @ le(imm)
	and {dst: datareg} {src: datareg} ^{imm: u16} =>
		(pc & 2 != 0 ? asm {pad} : 0`0) @ 0x3E @ {dst} @ {src} @ le(imm)
	ior {dst: datareg} {src: datareg} {imm: u16}  =>
		(pc & 2 != 0 ? asm {pad} : 0`0) @ 0x3F @ {dst} @ {src} @ le(imm)
	ior {dst: datareg} {src: datareg} ^{imm: u16} =>
		(pc & 2 != 0 ? asm {pad} : 0`0) @ 0x40 @ {dst} @ {src} @ le(imm)
	xor {dst: datareg} {src: datareg} {imm: u16}  =>
		(pc & 2 != 0 ? asm {pad} : 0`0) @ 0x41 @ {dst} @ {src} @ le(imm)
	xor {dst: datareg} {src: datareg} ^{imm: u16} =>
		(pc & 2 != 0 ? asm {pad} : 0`0) @ 0x42 @ {dst} @ {src} @ le(imm)

	in  {tgt: datareg} {addr: datareg} {imm: s16} =>
		(pc & 2 != 0 ? asm {pad} : 0`0) @ 0x43 @ {tgt} @ {addr} @ le(imm)
	out {tgt: datareg} {addr: datareg} {imm: s16} =>
		(pc & 2 != 0 ? asm {pad} : 0`0) @ 0x44 @ {tgt} @ {addr} @ le(imm)
}

#ruledef extras {
	hlt => 0x00_00`16
	pad => 0x00_FF`16
}

#ruledef pseudo {
	jmp {gpr: datareg} => asm { jmp {gpr} if !c0}
	jmp {gpr: addrreg} => asm { jmp {gpr} if !c0}
	jmp ip {gpr: datareg} => asm { jmp ip {gpr} if !c0}
	jmp ip {gpr: addrreg} => asm { jmp ip {gpr} if !c0}

	add {tgt: datareg} {imm: s16}  => asm { add {tgt} {tgt} {imm} }
	add {tgt: addrreg} {imm: s16}  => asm { add {tgt} {tgt} {imm} }
	add {tgt: datareg} ^{imm: s16} => asm { add {tgt} {tgt} ^{imm} }
	add {tgt: addrreg} ^{imm: s16} => asm { add {tgt} {tgt} ^{imm} }

	and {tgt: datareg} {imm: u16}  => asm { and {tgt} {tgt} {imm} }
	and {tgt: datareg} ^{imm: u16} => asm { and {tgt} {tgt} ^{imm} }
	ior {tgt: datareg} {imm: u16}  => asm { ior {tgt} {tgt} {imm} }
	ior {tgt: datareg} ^{imm: u16} => asm { ior {tgt} {tgt} ^{imm} }
	xor {tgt: datareg} {imm: u16}  => asm { xor {tgt} {tgt} {imm} }
	xor {tgt: datareg} ^{imm: u16} => asm { xor {tgt} {tgt} ^{imm} }

	in {tgt: datareg} {imm: s16} => asm { in {tgt} x0 {imm} }
	out {tgt: datareg} {imm: s16} => asm { out {tgt} x0 {imm} }
}

#ruledef macro {
	li  {dst: datareg} {imm: i32} => asm {
		ior {dst} x0 {imm}[15:0]
		ior {dst} ^{imm}[31:16]
	}
}

#fn lenstr(str) => le(strlen(str)`32) @ str

#bankdef main {
	#addr 0x00000000
	#size 0x400
	#outp 0
	#bits 8
}

#bank main
