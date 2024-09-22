#addr 0

out x1 0
add x1 1

out x1 0
add x1 1

out x1 0
add x1 1

out x1 0

wait:
	in  x1 0
	add x1 -256
	tst nzr c1 x1
jmp ip wait if c1
hlt
