#addr 0

li x2 wait

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
jmp x2 if c1
hlt
