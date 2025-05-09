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

#fn w_gg_ih(tgt_g, src_g, imm16) => src_g`4 @ tgt_g`4 @ imm16`16

; ---------------------------------------------------------------------------- ;

#ruledef native_w_cc3g {
	inp {dg: data} [ {sg: data} {ih: u16} ] => 0x20`8 @ w_gg_ih({dg}, {sg}, {ih})
	out {dg: data} [ {sg: data} {ih: u16} ] => 0x21`8 @ w_gg_ih({dg}, {sg}, {ih})
}
