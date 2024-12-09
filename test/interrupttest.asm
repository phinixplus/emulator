#addr 0
li  t0 userland
mov jp t0
li  t0 new_isr
out t0 isrloc
rsm

#addr 0x20
new_isr:
in  at irqsrc
out at irqsrc
out zr dbgcon
rsm

#addr 0x40
userland:
li  t0 1
out t0 dbgcon
; fire an interrupt with debugger here
brk
out zr envcmd
