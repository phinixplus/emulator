## PHINIX+ Emulator v0.1.0: TTY Update
This is the first major bundled release for the emulator project.
With it come very substantial updates to the project when it comes to
user interface. No longer is the serial port emulation just a bodge.
This update has been in the works for more than three months by now
and I am very happy to finally be able to show you.

### Changes
-	Reimplemented the serial port emulation; still using Telnet.
	-	Runs on a separate thread so as to not hinder CPU execution.
	-	Able to support up to 32 concurrent users on the one thread.
	-	Configurable maximum users and server port over CLI arguments.
	-	Runtime-configurable FIFO buffer interrupt thresholds.
-	Rewrote the IO FIFO component to overcome thread synchronization issues.
-	Rewrote the project Makefile in order to allow easier future expansion.
-	Fresh bundle of example assembly programs to test out the new peripheral.

### Tweaks
-	Minor bugfixes on the ISA definition file.
-	Rearranged header includes to be in four groups, alphabetically.
-	Added some bit-field utilities for I/O device implementations.
-	Added a FIFO used-space query function in addition to the free-space one.
-	Started using condition variables properly; BRK instruction rewrite.
-	Did initialization booleans properly, using `atomic_bool`.
-	CPU state printing now fully in `stderr` instead of partly.

---

You can pick up the trial package, `pplusemu_v0.1.0_trial.zip`,
if you just want to check the emulator out and don't want to
write assembly language yourself. It contains the static binary
along with pre-assembled example programs.

Alternatively, you can get the development package,
`pplusemu_v0.1.0_develop.zip`, if you want to write and
assemble programs for the emulator yourself. It contains
the architecture definitions and the 3rd-party assembler,
`customasm` as a static binary.
