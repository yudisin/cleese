# $Id$

OBJS=main.o
OUT=helloworld

include ../../makefile.all

LIBS=-lusr -lc

helloworld: $(OBJS)
	$(LD) $(LDFLAGS) -o helloworld $(CRT0) $(OBJS) $(LIBS)

install: all
	strip helloworld
	cp helloworld $(ROOT)/boot
