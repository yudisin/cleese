.SUFFIXES:	.boa

LDSCRIPT = cleese.ld
CC       = gcc -D__KERNEL__ -I ./include -I ./kernel/include -Wall -O2 -nostdinc -fno-builtin #-DVERBOSE_MALLOC -DVERBOSE_MEMCPY -DVERBOSE_MARSHAL
LD       = ld -T $(LDSCRIPT) -nostdlib

KERNEL_OBJS =	\
		kernel/lib/kprintf.o \
		kernel/src/mach/x86/main.o \
		kernel/src/kernel_bytecode.o

MACH_OBJS =	\
		kernel/src/mach/x86/clock.o \
		kernel/src/mach/x86/console.o \
		kernel/src/mach/x86/cpu.o \
		kernel/src/mach/x86/hal.o \
		kernel/src/mach/x86/keyboard.o \
		kernel/src/mach/x86/malloc.o \
		kernel/src/mach/x86/memory.o \
		kernel/src/mach/x86/pic.o \
		kernel/src/mach/x86/rtc.o \
		kernel/src/mach/x86/speaker.o \
		kernel/src/mach/x86/thread.o

LIB_OBJS =	\
		lib/string.o \
		lib/snprintf.o

OBJECT_OBJS =	\
		python/Objects/frameobject.o \
		python/Objects/intobject.o \
		python/Objects/dictobject.o \
		python/Objects/stringobject.o \
		python/Objects/typeobject.o \
		python/Objects/object.o \
		python/Objects/tupleobject.o \
		python/Objects/moduleobject.o \
		python/Objects/methodobject.o \
		python/Objects/abstract.o \
		python/Objects/classobject.o \
		python/Objects/funcobject.o \
		python/Objects/boolobject.o

PYTHON_OBJS =	\
		python/Python/pythonrun.o \
		python/Python/pystate.o \
		python/Python/import.o \
		python/Python/marshal.o \
		python/Python/ceval.o \
		python/Python/compile.o \
		python/Python/bltinmodule.o \
		python/Python/getargs.o \
		python/Python/modsupport.o

MODULE_OBJS =	\
		python/Modules/gcmodule.o \
		python/Modules/mach/isrmodule.o

OBJS =		\
		$(KERNEL_OBJS) \
		$(MACH_OBJS) \
		$(LIB_OBJS) \
		$(OBJECT_OBJS) \
		$(PYTHON_OBJS) \
		$(MODULE_OBJS)

all: c.img

clean:
	-rm -f KERNEL.BIN
	-rm -f kernel.lst kernel.map
	-rm -f kernel/src/kernel_bytecode.c
	find . -name '*.o' -exec rm -f {} ';'
	find . -name '*.s' -exec rm -f {} ';'

clobber: clean
	-rm -f bochsout.txt
	-rm -f disk/*.img
	find . -name '*~' -exec rm -f {} ';'

kernel/src/kernel_bytecode.c: kernel/src/kernel.py
	(cd kernel/src && python ../../util/freeze.py kernel.py)

kernel/src/mach/x86/locore.o: kernel/src/mach/x86/locore.S
	$(CPP) ./include -I ./kernel/include kernel/src/mach/x86//locore.S > kernel/src/mach/x86//locore.s
	$(AS) -o kernel/src/mach/x86/locore.o kernel/src/mach/x86/locore.s

.c.o:
	$(CC) -c -I./include -I./python/Include -o $@ $<

.boa.o:
	python util/boa < $< | as -o $@

c.img: KERNEL.BIN
	MTOOLSRC=disk/mtools.conf mtools -c mcopy -o $< C:

KERNEL.BIN: kernel/src/mach/x86/kstart.o kernel/src/mach/x86/locore.o $(OBJS) $(LDSCRIPT) 
	$(LD) -Map kernel.map -o $@ kernel/src/mach/x86/kstart.o kernel/src/mach/x86/locore.o $(OBJS)
	nm $@ | sort > kernel.lst
	strip $@
