#subruledef data {
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
}

#subruledef addr {
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
}

#subruledef cond {
	c0 => 0x0`4
	c1 => 0x1`4
	c2 => 0x2`4
	c3 => 0x3`4
	c4 => 0x4`4
	c5 => 0x5`4
	c6 => 0x6`4
	c7 => 0x7`4
	!{base: cond} =>
		({base} ^ 8)`4
}

; ---------------------------------------------------------------------------- ;

#subruledef flag {
	C => 0`2
	K => 1`2
	!{base: flag} =>
		({base} ^ 2)`2
}

#subruledef mix {
	ior => 0`1
	and => 1`1
}

#fn w_cc3g(funct, tgt_g, src1_g, src2_g, nt, tgt_c, ns, src_c) =>
	src1_g`4 @ tgt_g`4 @ src2_g`4 @ funct`4 @ ns`1 @ src_c`3 @ nt`1 @ tgt_c`3
#fn w_g_il(tgt_g, imm20) => imm20[19:16] @ tgt_g`4 @ imm20[15:0]
#fn w_gg_ih(tgt_g, src_g, imm16) => src_g`4 @ tgt_g`4 @ imm16`16

; ---------------------------------------------------------------------------- ;

#ruledef native_wg_il {
	jnl {dg: data} [ ip {lbl: u32} ] => {
		rel = ({lbl} - $) >> 1
		assert($[0:0] == 0 && {lbl}[0:0] == 0, "Misaligned jump target.")
		assert(rel[31:20] == 0 || rel[31:20] == 0xFFF, "Jump target out of range.")
		0x10`8 @ w_g_il({dg}, rel)
	}
}

#ruledef native_w_gg_ih {
	inp {dg: data} [ {sg: data} {ih: u16} ] => 0x20`8 @ w_gg_ih({dg}, {sg}, {ih})
	out {dg: data} [ {sg: data} {ih: u16} ] => 0x21`8 @ w_gg_ih({dg}, {sg}, {ih})

	add {dg: data} {sg: data} {ih: i16} => 0x22`8 @ w_gg_ih({dg}, {sg}, {ih})
}

#ruledef native_w_cc3g {
	add {dg: data} {s1g: data} {s2g: data}, {m: mix} {dc: cond} {sc: cond} {f: flag} => {
		assert({dc}[3:3] == 0, "Unimplemented pseudo-instruction.")
		0x40`8 @ w_cc3g({f}[0:0] @ {m}, {dg}, {s1g}, {s2g}, {f}[1:1], {dc}, {sc}[3:3], {sc})
	}
	sub {dg: data} {s1g: data} {s2g: data}, {m: mix} {dc: cond} {sc: cond} {f: flag} => {
		assert({dc}[3:3] == 0, "Unimplemented pseudo-instruction.")
		0x41`8 @ w_cc3g({f}[0:0] @ {m}, {dg}, {s1g}, {s2g}, {f}[1:1], {dc}, {sc}[3:3], {sc})
	}
}
