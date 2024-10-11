#!/bin/bash

bindir='build'

function error {
	echo "This build option requires $1."
	exit
}

function assemble {
	customasm --version >/dev/null || error "customasm"

	mkdir -p "$bindir"
	customasm "archdef/archdef_emu.asm" "$1" -f logisim8 -o "$bindir/mem.hex"
}

function build {
	make -j $(nproc) $1
}

function clean {
	[[ -d "$bindir/" ]] && rm -r "$bindir/"
	return 0
}

case $1 in
	"assemble") assemble $2 ;;
	"build") build $2 ;;
	"clean") clean ;;
esac
