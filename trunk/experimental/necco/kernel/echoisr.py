import isr
import vga
import keyb
import rtc

tb = vga.textbuffer

def kbd_isr():
	while keyb.more_chars():
	    ch = keyb.translate_scancode(keyb.get_scancode())
	    if ch:
		print ch
		tb[0] = ch; tb[1] = '\015'
	print rtc.seconds()

def clk_isr():
	tb[158] = '/-\|'[isr.ticker & 3]

isr.setvec(clk_isr, kbd_isr)

while 1:
	pass
