#!/bin/bash

bindir='build'
srcdir='source'
incldir='include'
outfile='pplusemu'

function build {
	case $1 in
		# So far the minimum viable standard is C99
		"release") GCC_ARGS="-Wall -Wextra -Werror -pedantic --std=c99 -O2" ;;
		"debug") GCC_ARGS="-Wall -Wextra -pedantic --std=c99 -g" ;;
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
	| while read -r srcfile ; do
		filename=$(echo "$srcfile" | xargs basename | cut -d'.' -f1)
		this_binfile="$this_bindir/$filename.o"
		
		mkdir -p "$this_bindir"
		echo "Building: $srcfile -> $this_binfile"
		gcc $GCC_ARGS -c -o "$this_binfile" $srcfile -Iincl -I"$this_incldir"
	done

	# Recurse for all subdirectories and then merge generated object files
	find $1 -maxdepth 1 -mindepth 1 -type d \
	| while read -r srcdir ; do
		build_rec "$srcdir"
		this_bindir=$(echo "$srcdir" | sed -e 's/src/bin/')
		binfiles=$(find $this_bindir -maxdepth 1 -mindepth 1 -type f -name "*.o")
		binfiles=$(echo "$binfiles" | tr '\n' ' ')
		echo "Merging: $binfiles -> ${this_bindir}_dir.o"
		ld --relocatable -o "${this_bindir}_dir.o" $binfiles
	done
}

function clean {
	[[ -d bin/ ]] && rm -r bin/
	return 0
}

case $1 in
	"build") build $2 ;;
	"clean") clean ;;
esac
