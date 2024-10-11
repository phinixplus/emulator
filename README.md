# PHINIX+ Emulator
This is the repository containing the standard PHINIX+ system emulator. \
The emulator itself is written using C and its standard library for Linux.

## Building
You will need `make` to be able to compile the emulator and/or the
provided example assembly programs.

Make sure `customasm` and `gcc` are installed and available in your PATH
in order to make full use of the provided makefile, or substitute your
own if need be. Compatibility not guaranteed with other tools.

To compile the emulator sources into an executable run
`make debug` or `make release` depending on your desired use case.
After a successful build the executable should be available at
`build/pplusemu`.

After building the emulator, run make again, this time to build
the provided example programs using the command `make assemnble`.
This will create hexadecimal memory files at `build/asm/`. Pick one
that you would like to test and run it using the emulator with the
command `./build/pplusemu build/asm/<asm-file>`.

### Cleanup
You can run `make clean` to remove build artifacts but retain the final
executables and assembled example programs.

You can also run `make purge` to completely remove all programmatically
generated files.

## Assembly Programming
Currently, the PHINIX+ assembly is defined using the
[CustomASM](https://github.com/hlorenzi/customasm) utility.
The list of available instructions and their operands are declared in
[archdef/ppluscpu.asm](archdef/ppluscpu.asm). To learn more
about the PHINIX+ system architecture, read the
[documentation](https://github.com/phinixplus/docs).
