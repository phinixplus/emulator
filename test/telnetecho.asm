#addr 0

loop:
	in t0 telnet
	tst neg c1 t0
	jmp ip loop if c1
	out t0 telnet
jmp ip loop
