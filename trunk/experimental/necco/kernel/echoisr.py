import pyvga
import vga

########################################

def decorate():
	o = 61114	# (320*190) + (320-6)
	y = 0
	fb = vga.framebuffer
	while y < 10:
		if (y & 1):	fb[o:o+6] = '\077\067\077\067\077\067'
		else:		fb[o:o+6] = '\067\077\067\077\067\077'
		o = o + 320
		y = y + 1

def splashup():
	pyvga.savetext()
	pyvga.set320x200x256()
	# why doesn't framebuffer[:] also work?
	vga.framebuffer[:0x10000] = vga.splashscreen
	decorate()

splashup()

def pause():
	timeout = rtc.seconds() + 2
	while rtc.seconds() < timeout:
		pass

def splashdown():
	pause()
	pyvga.set80x25()
	pyvga.restoretext()

########################################

import isr
import keyb
import rtc

tb = vga.textbuffer

def kbd_isr():
	while keyb.more_chars():
	    ch = keyb.translate_scancode(keyb.get_scancode())
	    if ch:
		print ch
		tb[0] = ch; tb[1] = '\015'

def clk_isr():
	tb[158] = '/-\|'[isr.ticker & 3]

isr.setvec(clk_isr, kbd_isr)

########################################

splashdown()
print 'Press a key'
while 1:
	pass
