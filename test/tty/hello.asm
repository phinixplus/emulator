#addr 0
brk ; Wait for it to connect

; TTY0 Read Thres = 0 (IRQ on any)
li  t0 0b10_00000
out t0 ttyreq
out zr ttystat

; TTY0 Write Thres = message length
li  t0 0b11_00000
out t0 ttyreq
li  at message
lw  t0 at
out t0 ttystat

in  t0 irqsrc
out t0 irqsrc

program:
	brk ; Wait for next IRQ

	; Exit if it disconnected
	; or if a second one connected
	in  t0 ttycon
	sub t0 1
	tst nzr c1 t0
	jmp ip exit if c1

	; Print the message
	li  at message
	mov t3 at
	lw  t1 t3
	add t3 4
	.loop:
		lbu at t3
		out at ttydata
		add t3 1
		sub t1 1
		tst zer c1 t1
	jmp ip .loop if !c1

	; Clear all interrupts
	in  t0 irqsrc
	out t0 irqsrc

	jmp ip program
exit:
out zr envcmd ; Shutdown

#align 32
message:
#d lenstr("Hello, world! ")
