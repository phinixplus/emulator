#addr 0

li t0 46
li t1 1
li t2 0
loop:
	mov at t2
	add at t1
	out at dbgcon
	mov t1 t2
	mov t2 at
sub t0 1
tst nzr c1 t0
jmp ip loop if c1

wait:
	in  at dbgcon
	add at -256
	tst nzr c1 at
jmp ip wait if c1
out zr envcmd
