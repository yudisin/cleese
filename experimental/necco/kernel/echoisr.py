########################################
import pyvga

def splashup():
	pyvga.savetext()
	pyvga.set320x200x256()
	pyvga.framebuffer[:0xFA00] = pyvga.splashscreen[:0xFA00]

splashup()

def decorate():
	o = 61114	# (320*190) + (320-6)
	fb = pyvga.framebuffer
	y = 0
	while y < 10:
		if (y & 1):	fb[o:o+6] = '\077\067\077\067\077\067'
		else:		fb[o:o+6] = '\067\077\067\077\067\077'
		o = o + 320
		y = y + 1

decorate()

def splashdown():
	pyvga.set80x25()
	pyvga.restoretext()

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

isr.setvec(clk_isr, kbd_isr)

########################################

while isr.ticker < 60: pass
splashdown()

print 'Press a key'
while 1:
	pass
