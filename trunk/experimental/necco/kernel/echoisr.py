import pyvga

def splashup():
	pyvga.exittext()
	pyvga.framebuffer[:0xFA00] = pyvga.splashscreen[:0xFA00]
	pyvga.cleartext()

def splashdown():
	pyvga.entertext()

splashup()

########################################
import pyfont
pyfont.setctx(pyvga.framebuffer, pyvga.font0)
pyfont.write(12,12, 'hello, world!')
########################################

import isr
import py8042
import keyb

tb = pyvga.textbuffer

def prch(ch, fmt):
	print ch
	tb[0] = ch; tb[1] = fmt

def kbd_isr():
	while py8042.more_chars():
	    ch = keyb.translate_scancode(py8042.get_scancode())
	    if ch:
		prch(ch, '\015')

def clk_isr():
	tb[158] = '/-\|'[isr.ticker & 3]
	fl = py8042.poll_mouse()
	if fl:
		print "mouse: %x %d %d" % (fl, py8042.dx, py8042.dy)
		clk_isr()
		

isr.setvec(clk_isr, kbd_isr)

########################################

while (isr.ticker < 40) and (py8042.squeaks < 1):
	pass

splashdown()
print 'Press a key'

while 1:
	pass
