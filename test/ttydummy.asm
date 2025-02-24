#addr 0 ; Reset code
program:
	brk
	in  t0 ttycon
	tst zer c1 t0
	jmp ip exit if c1

	in  t0 irqsrc
	out t0 irqsrc
jmp ip program
exit:
out zr envcmd
