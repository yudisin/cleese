.SUFFIXES: .asm

LDSCRIPT = krnl1m.ld
NASM     = nasm -f elf
CC       = gcc -Wall -O2 -nostdinc -fno-builtin #-DVERBOSE_MALLOC -DVERBOSE_MEMCPY
LD       = ld -T $(LDSCRIPT) -nostdlib

OBJECT_OBJS =	\
		Objects/frameobject.o \
		Objects/intobject.o \
		Objects/dictobject.o \
		Objects/stringobject.o \
		Objects/typeobject.o \
		Objects/object.o \
		Objects/tupleobject.o \
		Objects/moduleobject.o

PYTHON_OBJS =	\
		Python/pythonrun.o \
		Python/pystate.o \
		Python/import.o \
		Python/marshal.o \
		Python/ceval.o \
		Python/compile.o

MODULE_OBJS =	\
		Modules/main.o \
		Modules/M___main__.o \
		Modules/gcmodule.o

LIB_OBJS =	\
		lib/string.o \
		lib/mem.o

MACH_OBJS =	\
		mach/video.o \
		mach/ports.o \
		mach/malloc.o

OBJS =	\
	$(OBJECT_OBJS) \
	$(PYTHON_OBJS) \
	$(MODULE_OBJS) \
	$(LIB_OBJS) \
	$(MACH_OBJS)

all: KERNEL.BIN

clean:
	-rm -f *.o KERNEL.BIN kernel.lst Objects/*.o Python/*.o Modules/*.o lib/*.o mach/*.o

.asm.o:
	$(NASM) -o $@ $<

.c.o:
	$(CC) -c -I./Include -o $@ $<

KERNEL.BIN: kstart.o Modules/python.o $(OBJS) $(LDSCRIPT) 
	$(LD) -o $@ kstart.o Modules/python.o $(OBJS)
	nm $@ | sort > kernel.lst
	strip $@






