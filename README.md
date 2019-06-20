# About

This repostiroy contains microcontroller code for the escape game system.
Microcontroller codes have not been spit into separate repositories due to
the very limited code size and similar codebase.

# Building and flashing

## Dependencies

The microcontroller code has by itself no library dependencies, apart from (if
you can even count that) the libc of your toolchain.

The microcontroller code should compile with almost any version of the AVR-GCC.
Flashing requires the AVRDUDE to be installed. Executeing the make file reqires
GNUE MAKE.

## Settings

There are several different settings which should be checked before compiling
the code:

- The ECP-Device-ID in the **include/general.h** is set to the targeted id.

- The targeted **BOARD**, **PROGRAMMER**, **PROGBOARD** and **PORT** are set for
  your usecase in the Makefile.

- The **correct** command-line is commented out in the **flash:** target of
  the Makefile.

- Any extra options needed are appended to the **CFLAGS** variable of the 
  Makefile.

### Compilation flags

Preprocessor defines may be appended to the CFLAGS variable inside the Makefile,
these are specific to to the microcontroller codebase and may be documented in
the README of saied.

## Compiling

All .c files in the srcdirectory are compiled to .o files, then linked together
to form an elf binary which is transformed by objcopy to an intel-hex file. This
can be done by simply typing in the make command or by running the command with
the target **make all**. 

In case any of the settings has been changed it is recommended to first delete
all old builds by running the command **make clean** and then building it again.

## Flashing

After setting everything up correctly you can run the **make flash** command
to execute the flash commandline. The flash command will write the flash to
the device twice.

