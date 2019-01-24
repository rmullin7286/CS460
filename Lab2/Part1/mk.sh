# 1. What does the following statement do?
# A: This statement assembles the ts.s file using the Arm
# "Embedded Application Binary Interface".
# An application binary interface (ABI) determines how functions should be called,
# the syscalls used in the assembly, etc. Each architecture and OS has it's own ABI. 
# This file is assembled for the ARM926EJ-S CPU
arm-none-eabi-as -mcpu=arm926ej-s -g ts.s -o ts.o

# 2. What does the following statement do?
# A: This compiles the t.c C file using the same ABI as the first command
arm-none-eabi-gcc -c -mcpu=arm926ej-s -g t.c -o t.o

# 3. What does the following statement do?
# A: ld is a linker that links multiple object files into a single binary executable.
# The output of the linker on a standard Unix based system is an elf file
# The elf file contains the compiled binary, but is not yet executable.
arm-none-eabi-ld -T t.ld ts.o t.o /usr/lib/arm-none-eabi/lib/libc.a /usr/lib/gcc/arm-none-eabi/5.4.1/libgcc.a -o t.elf


# 4. What does the following statement do?
# A: objcopy copies the binary contents of the elf file to an executable binary file
arm-none-eabi-objcopy -O binary t.elf t.bin

rm *.o *.elf

echo ready to go?
read dummy

qemu-system-arm -M versatilepb -m 128M -kernel t.bin \
-serial mon:stdio
 
