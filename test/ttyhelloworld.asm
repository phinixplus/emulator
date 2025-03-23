#addr 0 ; Reset code
brk
li  at handle
mov s6 at
program:
	in  s0 ttycon
	tst zer c1 s0
	jmp ip exit if c1

	li s1 31
	.loop:
		and at s0 1
		tst zer c1 at
		jmp ip .skip if c1
		jnl rp s6
		.skip:
		tst zer c1 s1
		sub s1 1
		sru s0 s0
	jmp ip .loop if !c1

	in  t0 irqsrc
	out t0 irqsrc
jmp ip program
exit:
out zr envcmd

handle:
add t0 zr 63
sub t0 s1
out t0 ttyreq

li  at message
mov t3 at
lw  t1 t3

in  t0 ttystat
sub t0 t1
tst neg c1 t0
jmp rp if c1

add t3 4
.loop:
	lbu  at t3
	out  at ttydata
	add t3 1
	sub t1 1
	tst zer c1 t1
jmp ip .loop if !c1
jmp rp

#align 32
message:
#d lenstr("Hello, world! ")
