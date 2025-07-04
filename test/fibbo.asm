#addr 0

add x1 x0 1
add x2 x0 0

loop:
	add x3 x2 x1 , ior c1 c0 V
	out x3 [ x0 dbgcon ]
	add x1 x2 0
	add x2 x3 0
jmp [ ip x0 loop ] if c1

out x0 [ x0 envcmd ]
