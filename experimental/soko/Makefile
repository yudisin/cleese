.SUFFIXES:	.img .zip

PLAT	= bochs
OBJCOPY	= objcopy -R .note -R .comment -S
SRCDIRS = include lib python kernel util

all: dirs grub

clean:
	cd $(PLAT) && $(MAKE) clean

clobber: clean
	-rm -f bochsout.txt KERNEL.BIN
	-rm -f disk/*.img
	-rm -rf $(SRCDIRS)
	find . -name '*~' -exec rm -f {} ';'

.zip.img:
	unzip -d disk $<
	touch $@

cleese: FORCE
	cd $(PLAT) && $(MAKE) PLAT=$(PLAT)

grub: cleese disk/c.img disk/grub.img
	$(OBJCOPY) $(PLAT)/cleese KERNEL.BIN
	util/push KERNEL.BIN $@

bootp: cleese disk/c.img disk/bootp.img
	$(OBJCOPY) -O binary $(PLAT)/cleese KERNEL.BIN
	util/push KERNEL.BIN $@

FORCE:

###############################################################################
# do some quick copying so this won't waste space on sourceforge.

include:
	cp -r ../necco/$@ .
lib:
	cp -r ../necco/$@ .
python:
	cp -r ../necco/$@ .
kernel:
	cp -r ../necco/$@ .
util:
	cp -r ../necco/$@ .

dirs:	$(SRCDIRS)
	-rm -f kernel/kernel_bytecode.c
	find $^ -name '*.a' -exec rm -f {} ';'
	find $^ -name '*.o' -exec rm -f {} ';'
	find $^ -name '*.s' -exec rm -f {} ';'

