.SUFFIXES:	.boa

PYTHON	 = python
CC       = gcc -Wall -O2 -nostdinc -fno-builtin 
#CC	+= -DVERBOSE_MALLOC -DVERBOSE_MEMCPY -DVERBOSE_MARSHAL
LD       = ld -nostdlib

KERNEL_PY = \
		kernel/echoisr.py \
		kernel/keyb.py \
		kernel/rtc.py

KERNEL_OBJS =	\
		kernel/main.o \
		kernel/kernel_bytecode.o

KERNEL_OBJS +=	\
		kernel/bochs/init.o \
		kernel/bochs/israbc.o \
		kernel/bochs/isrmodule.o \
		kernel/bochs/portsmodule.o \
		kernel/bochs/vgaabc.o \
		kernel/bochs/vgadata.o \
		kernel/bochs/vgamodule.o

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

PYTHON_OBJS +=	\
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
		python/Objects/boolobject.o \
		python/Objects/listobject.o \
		python/Objects/bufferobject.o

PYTHON_OBJS +=	\
		python/Modules/gcmodule.o

LDSCRIPT =	lib/bochs/cleese.ld
CRT0	 =	lib/bochs/crt0.o
LIB_OBJS =	\
		lib/string.o \
		lib/malloc.o
LIB_OBJS +=	\
		lib/bochs/video.o \
		lib/bochs/ports.o \
		lib/bochs/mem.o

OBJS =		\
		$(KERNEL_OBJS) \
		$(PYTHON_OBJS) \
		$(LIB_OBJS) 

all: grub

clean:
	-rm -f cleese cleese.map
	find . -name '*.o' -exec rm -f {} ';'
	find . -name '*.s' -exec rm -f {} ';'

clobber: clean
	-rm -f bochsout.txt
	-rm -f disk/*.img
	find . -name '*~' -exec rm -f {} ';'

.c.o:
	$(CC) -c -I./include -I./python/Include -o $@ $<

.boa.o:
	$(PYTHON) util/boa < $< | as -o $@

kernel/kernel_bytecode.c: $(KERNEL_PY)
	$(PYTHON) util/freeze.py $^
	mv kernel_bytecode.c $@

cleese: $(LDSCRIPT) $(CRT0) $(OBJS) # order is important!
	$(LD) -Map cleese.map -o $@ -T $^
	strip $@

grub: cleese FORCE
	cp $< KERNEL.BIN
	MTOOLSRC=disk/mtools.conf mtools -c mcopy -o KERNEL.BIN C:
	(cd disk; ln -fs $@.img a.img)
	rm -f KERNEL.BIN

bootp: cleese FORCE
	objcopy -R .note -R .comment -S -O binary $< KERNEL.BIN
	MTOOLSRC=disk/mtools.conf mtools -c mcopy -o KERNEL.BIN C:
	(cd disk; ln -fs $@.img a.img)
	rm -f KERNEL.BIN

FORCE:
