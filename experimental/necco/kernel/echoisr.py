import isr
import vga
import keyb

tb = vga.textbuffer
ticks = 0

def kbd_isr():
	while keyb.more_chars():
	    ch = keyb.translate_scancode(keyb.get_scancode())
	    if ch:
		print ch
		tb[0] = ch; tb[1] = '\015'

def clk_isr():
	tb[158] = '01234567'[ticks & 7]
	vga.framebuffer[ticks & 0xfff] = '\377'

isr.setvec()

while 1:
	ticks = ticks + 1
	if not (ticks & 0xfff):
		if (ticks & 0x1000): vga.videomode(1)
		else:		     vga.videomode(0)
