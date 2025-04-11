#!/usr/bin/env bash
[ -z "$1" ] && echo "$0 <input> <output>" && exit 1
FLAGS='-q -flogisim8 archdef/archdef_emu.asm'
if [ -z "$2" ]; then ./customasm $FLAGS $1
else ./customasm $FLAGS $1 -o $2; fi
