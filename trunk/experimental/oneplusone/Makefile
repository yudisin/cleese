all: 1p1.exe

clean:
	find . -name '*.o' -exec rm -f {} ';'
	-rm -f *.da *.bb *.bbg *.gcov
	-rm -f 1p1.exe

OBJECT_OBJS =	\
		Objects/frameobject.o \
		Objects/intobject.o \
		Objects/dictobject.o \
		Objects/stringobject.o \
		Objects/typeobject.o \
		Objects/object.o \
		Objects/tupleobject.o \
		Objects/classobject.o \
		Objects/moduleobject.o

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

FLAGS = -fprofile-arcs -ftest-coverage -g 

1p1.exe: Modules/python.o $(OBJS)
	gcc $(FLAGS) -Wall -o $@ Modules/python.o $(OBJS)

.c.o:
	gcc $(FLAGS) -fno-builtin -c -Wall -I. -I./Include -o $@ $<
