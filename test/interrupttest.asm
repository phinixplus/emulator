#addr 0
li t0 userland
mov jp t0
li t0 new_isr
out t0 isrloc
jmp t0

#addr 0x20
new_isr:
out zr dbgcon
rsm

#addr 0x40
userland:
li t0 1
out t0 dbgcon
hlt
