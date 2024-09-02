#!/bin/bash

bindir='build'
srcdir='source'
incldir='include'
outfile='pplusemu'

function error {
	echo "This build option requires $1."
	exit
}

function assemble {
	customasm --version >/dev/null || error "customasm"

	mkdir -p "$bindir"
	customasm "pplusdef.asm" "$1" -f logisim8 -o "$bindir/mem.hex"
}

function build {
	gcc --version >/dev/null || error "gcc"
	ld --version >/dev/null || error "ld"

	case $1 in
		"release") GCC_ARGS="-Wall -Wextra -Werror -pedantic --std=c11 -O2" ;;
		"debug") GCC_ARGS="-Wall -Wextra -pedantic --std=c11 -g" ;;
	esac
	build_rec $srcdir
	binfiles=$(find $bindir -maxdepth 1 -mindepth 1 -type f -name "*.o")
	binfiles=$(echo "$binfiles" | tr '\n' ' ')
	echo "Executable: $binfiles -> $bindir/$outfile"
	gcc $GCC_ARGS -o "$bindir/$outfile" $binfiles
}

function build_rec {
	local this_incldir=$(echo "$1" | sed -e "s/$srcdir/$incldir/")
	local this_bindir=$(echo "$1" | sed -e "s/$srcdir/$bindir/")

	# Find and compile all C files in the current directory
	find $1 -maxdepth 1 -mindepth 1 -type f -name "*.c" \
	| while read -r this_srcfile ; do
		filename=$(echo "$this_srcfile" | xargs basename | cut -d'.' -f1)
		this_binfile="$this_bindir/$filename.o"
		
		mkdir -p "$this_bindir"
		echo "Building: $this_srcfile -> $this_binfile"
		gcc $GCC_ARGS -c -o "$this_binfile" $this_srcfile -I"$incldir" -I"$this_incldir"
	done

	# Recurse for all subdirectories and then merge generated object files
	find $1 -maxdepth 1 -mindepth 1 -type d \
	| while read -r this_srcdir ; do
		build_rec "$this_srcdir"
		this_bindir=$(echo "$this_srcdir" | sed -e "s/$srcdir/$bindir/")
		binfiles=$(find $this_bindir -maxdepth 1 -mindepth 1 -type f -name "*.o")
		binfiles=$(echo "$binfiles" | tr '\n' ' ')
		echo "Merging: $binfiles -> ${this_bindir}_dir.o"
		ld --relocatable -o "${this_bindir}_dir.o" $binfiles
	done
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
