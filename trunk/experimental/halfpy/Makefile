all: halfpy.exe

clean:
	find . -name '*.o' -exec rm -f {} ';'
	-rm -f halfpy.exe

UNICODE_OBJS = 	\
		Objects/unicodeobject.o \
		Objects/unicodectype.o

OBJECT_OBJS =	\
		Objects/abstract.o \
		Objects/boolobject.o \
		Objects/bufferobject.o \
		Objects/cellobject.o \
		Objects/classobject.o \
		Objects/cobject.o \
		Objects/complexobject.o \
		Objects/descrobject.o \
		Objects/dictobject.o \
		Objects/enumobject.o \
		Objects/fileobject.o \
		Objects/floatobject.o \
		Objects/frameobject.o \
		Objects/funcobject.o \
		Objects/intobject.o \
		Objects/iterobject.o \
		Objects/listobject.o \
		Objects/longobject.o \
		Objects/methodobject.o \
		Objects/moduleobject.o \
		Objects/object.o \
		Objects/obmalloc.o \
		Objects/rangeobject.o \
		Objects/sliceobject.o \
		Objects/stringobject.o \
		Objects/structseq.o \
		Objects/tupleobject.o \
		Objects/typeobject.o \
		Objects/weakrefobject.o \
		$(UNICODE_OBJS)

PYTHON_OBJS =	\
		Python/pythonrun.o \
		Python/ceval.o \
		Python/marshal.o \
		Python/getargs.o \
		Python/errors.o \
		Python/exceptions.o \
		Python/import.o \
		Python/importdl.o \
		Python/modsupport.o \
		Python/mysnprintf.o \
		Python/mystrtoul.o \
		Python/sysmodule.o \
		Python/codecs.o \
		Python/pystate.o \
		Python/structmember.o \
		Python/compile.o \
		Python/thread.o \
		Python/traceback.o \
		Python/bltinmodule.o \
		Python/dynload_shlib.o \
		Python/getversion.o \
		Python/getcopyright.o \
		Python/getcompiler.o \
		Python/getplatform.o \
		Python/frozen.o

MODULE_OBJS = 	\
		Modules/main.o \
		Modules/gcmodule.o \
		Modules/signalmodule.o \
		Modules/config.o \
		Modules/getbuildinfo.o \
		Modules/getpath.o

PARSER_OBJS =	\
		Parser/myreadline.o

OBJS = 		\
		$(OBJECT_OBJS) \
		$(PYTHON_OBJS) \
		$(MODULE_OBJS) \
		$(PARSER_OBJS)

halfpy.exe: Modules/python.o $(OBJS)
	gcc -Wall -o $@ Modules/python.o $(OBJS)

Objects/unicodectype.o: Objects/unicodectype.c Objects/unicodetype_db.h

.c.o:
	gcc -c -fno-strict-aliasing -DNDEBUG -g -O3 -Wall -I. -I./Include -DPy_BUILD_CORE -o $@ $<
