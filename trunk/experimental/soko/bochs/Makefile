CC       = gcc -Wall -O2 -nostdinc -fno-builtin
#CC	 += -fno-strict-aliasing
#CC      += -fno-leading-underscore
#CC	 += -DVERBOSE_MALLOC -DVERBOSE_MEMCPY -DVERBOSE_MARSHAL
LD       = ld -nostdlib
PYTHON	 = python

SRC	 = ..
include ../Make.ind

KERNEL_PY	= soko.py keyb.py pyvga.py py8042.py

KERNEL_OBJS +=	\
		init.o \
		isrmodule.o \
		portsmodule.o \
		vgadata.o

LIB_OBJS +=	\
		mem.o

LDSCRIPT =	$(SRC)/lib/$(PLAT)/cleese.ld
CRT0	 =	crt0.o israbc.o
HW_OBJS	 =	video.o \
		ports.o \

all: cleese

clean:
	-rm -f cleese cleese.map kernel_bytecode.c
	-rm -f *.a *.o *.s

kernel_bytecode.c: $(KERNEL_PY)
	$(PYTHON) $(SRC)/util/freeze.py $^

libpython.a:	$(PYTHON_OBJS)
	ar r $@ $?
libc.a:		$(LIB_OBJS)
	ar r $@ $?
libhw.a:	$(HW_OBJS)
	ar r $@ $?

cleese: $(LDSCRIPT) $(CRT0) $(KERNEL_OBJS) libpython.a libc.a libhw.a
	$(LD) -Map cleese.map -o $@ -T $^
