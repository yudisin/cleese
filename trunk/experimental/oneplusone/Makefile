all: 1p1.exe

clean:
	find . -name '*.o' -exec rm -f {} ';'
	-rm -f 1p1.exe

OBJECT_OBJS =	\
		Objects/frameobject.o \
		Objects/intobject.o \
		Objects/dictobject.o \
		Objects/stringobject.o \
		Objects/typeobject.o \
		Objects/object.o \
		Objects/tupleobject.o \
		Objects/boolobject.o \
		Objects/classobject.o \
		Objects/abstract.o \
		Objects/longobject.o \
		Objects/moduleobject.o \
		Objects/cellobject.o
#		Objects/obmalloc.o \
#		Objects/bufferobject.o \
#		Objects/cobject.o \
#		Objects/descrobject.o \
#		Objects/enumobject.o \
#		Objects/floatobject.o \
#		Objects/funcobject.o \
#		Objects/iterobject.o \
#		Objects/listobject.o \
#		Objects/methodobject.o \
#		Objects/rangeobject.o \
#		Objects/sliceobject.o \
#		Objects/structseq.o \
#		Objects/weakrefobject.o

PYTHON_OBJS =	\
		Python/pythonrun.o \
		Python/pystate.o \
		Python/import.o \
		Python/marshal.o \
		Python/ceval.o \
		Python/compile.o

MODULE_OBJS = 	\
		Modules/main.o \
		Modules/M___main__.o \
		Modules/gcmodule.o

OBJS = 		\
		$(OBJECT_OBJS) \
		$(PYTHON_OBJS) \
		$(MODULE_OBJS)

1p1.exe: Modules/python.o $(OBJS)
	gcc -g -Wall -o $@ Modules/python.o $(OBJS)

.c.o:
	gcc -g -fno-builtin -c -Wall -I. -I./Include -o $@ $<
