all: KERNEL.BIN

Modules/python.o: Modules/python.c
	gcc -ffreestanding -c -I./Include Modules/python.c -o Modules/python.o

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

.c.o:
	gcc -fno-builtin -Wall -c -I./Include -o $@ $<

kernel.test: Modules/python.o $(OBJS)
	ld -e _main -Ttext 0x1000 -o kernel.test Modules/python.o $(OBJS)

kernel.o: kernel.test
	ld -i -e _main -Ttext 0x1000 -o kernel.o Modules/python.o $(OBJS)

KERNEL.BIN: kernel.o
	objcopy -R .note -R .comment -S -O binary kernel.o KERNEL.BIN

clean:
	-rm -f *.o
	-rm -f Objects/*.o
	-rm -f Python/*.o
	-rm -f Modules/*.o
	-rm -f lib/*.o
	-rm -f mach/*.o
	-rm -f kernel.test
	-rm -f KERNEL.BIN
