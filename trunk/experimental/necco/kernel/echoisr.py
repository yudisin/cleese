import isr
import vga
import keyb
import rtc
import pyvga

########################################

def splashscreen():
	pyvga.set320x200x256()
	# why doesn't framebuffer[:] also work?
	vga.framebuffer[:0x10000] = vga.splashscreen
	while not (rtc.seconds() & 0x4):
		pass
	pyvga.set80x25()

splashscreen()
print 'Press a key'

########################################
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

t = 0
while 1:
	pass
	# print vga.splashscreen[t:t+16]
	# t = t + 16
