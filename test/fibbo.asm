#addr 0

add x1 x0 1
add x4 x0 256

loop:
	add x3 x2 x1 , ior c1 c0 V
	jmp [ ip exit x0 ] if c1
	out x3 [ x0 dbgcon ]
	add x1 x2 0
	add x2 x3 0
jmp [ ip loop x0 ]

exit:
	inp x1 [ x0 dbgcon ]
	sub x0 x1 x4 , ior c1 c0 !C
jmp [ ip exit x0 ] if c1
out x0 [ x0 envcmd ]
