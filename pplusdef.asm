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

#ruledef extras {
	hlt => 0x00_00_00_00`8
}

#ruledef half_reg_reg {
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

	slu {dst: datareg} {src: datareg} => 0x0E`8 @ {dst} @ {src}
	sru {dst: datareg} {src: datareg} => 0x0F`8 @ {dst} @ {src}
	srs {dst: datareg} {src: datareg} => 0x10`8 @ {dst} @ {src}
	blu {dst: datareg} {src: datareg} => 0x11`8 @ {dst} @ {src}
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

#bankdef main {
	#addr 0x00000000
	#size 0x400
	#outp 0
	#bits 8
}

#bank main
