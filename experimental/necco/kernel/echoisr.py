########################################
import pyvga

def splashup():
	pyvga.set320x200x256()
	pyvga.savevga()
	pyvga.savefonts()
	pyvga.framebuffer[:0xFA00] = pyvga.splashscreen[:0xFA00]

splashup()

def decorate(off):
	o = off
	fb = pyvga.framebuffer
	y = 0
	while y < 12:
		if (o & 64):	fb[o:o+6] = '\015\067\015\067\015\067'
		else:		fb[o:o+6] = '\067\015\067\015\067\015'
		o = o + 320
		y = y + 1

def splashdown():
	pyvga.restorevga()
	pyvga.restorefonts()
	pyvga.set80x25()

########################################

#import pyfont
#pyfont.write(12,12, 'hello, world!')

########################################

import isr
import keyb
import comport

tb = pyvga.textbuffer

def prch(ch, fmt):
	print ch
	tb[0] = ch; tb[1] = fmt
	if ord(ch) == 78:
		ch = '\n'
	comport.send(ch)

def kbd_isr():
	while keyb.more_chars():
	    ch = keyb.translate_scancode(keyb.get_scancode())
	    if ch:
		prch(ch, '\015')

def clk_isr():
	tb[158] = '/-\|'[isr.ticker & 3]
	pass

isr.setvec(clk_isr, kbd_isr)

########################################

o = 314
t = 0
while isr.ticker < 60:
	if isr.ticker > t:
		if o < 64000:
			decorate(o)
			o = o + 2560
			t = isr.ticker + 1

splashdown()

print 'Press a key'
while 1:
	pass
