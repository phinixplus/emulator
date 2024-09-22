#addr 0

helloworld:
li t0 stringus

lw t1 t0
add t0 4
loop:
	lbu t2 t0
	out t2 1
	add t0 1
	sub t1 1
	tst nzr c1 t1
jmp ip loop if c1

jmp zr

#align 32
stringus:
#d lenstr("Hello, world!\r\n")
