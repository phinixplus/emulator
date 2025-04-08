#addr 0
brk ; Wait for it to connect

; TTY0 Read Thres = 0 (IRQ on any)
li  t0 0b10_00000
out t0 ttyreq
out zr ttystat

; TTY0 Write Thres = 256 (Disabled)
li  t0 0b11_00000
out t0 ttyreq
li  t0 256
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

	; Echo the character
	in  t0 ttydata
	out t0 ttydata

	; Clear all interrupts
	in  t0 irqsrc
	out t0 irqsrc

	jmp ip program
exit:
out zr envcmd ; Shutdown
