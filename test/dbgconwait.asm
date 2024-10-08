#addr 0

out x1 dbgcon
add x1 1

out x1 dbgcon
add x1 1

out x1 dbgcon
add x1 1

out x1 dbgcon

wait:
	in  x1 dbgcon
	add x1 -256
	tst nzr c1 x1
jmp ip wait if c1
hlt
