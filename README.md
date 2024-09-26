# PHINIX+ Emulator
This is the repository containing the standard PHINIX+ system emulator. \
The emulator itself is written using C and its standard library for Linux.

## Building
Make sure `customasm`, `gcc`, and `ld` are installed and available
in order to make full use of the provided build script.

To compile the emulator sources into an executable run
`./build.sh build debug` or `./build.sh build release`
depending on your desired use case. After a successfull build
the executable should be available at `build/pplusemu`.

After building the emulator, run the build script again, this time
providing it a valid [CustomASM](https://github.com/hlorenzi/customasm)
assembly file using the command `./build.sh assemble <path-to-asm>`.
This will create a hexadecimal memory file at `build/mem.hex`.

Finally, run your assembly program using the command
`build/pplusemu build/mem.hex`.

## Assembly Programming
Currently, the PHINIX+ assembly is defined using the
[CustomASM](https://github.com/hlorenzi/customasm) utility.
The list of available instructions and their operands are declared in
[archdef/ppluscpu.asm](archdef/ppluscpu.asm). To learn more
about the PHINIX+ system architecture, read the
[documentation](https://github.com/phinixplus/docs).
