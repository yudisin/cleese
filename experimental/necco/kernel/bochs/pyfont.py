import vga
import blit

""" pure python text rendering:
    uses a subroutine call and a branch per pixel
    too slow for my bochs, but runs fine on hardware """

fb          = vga.framebuffer

def pixel(val, off):
	if val:	fb[off] = '\067'
	else:	fb[off] = '\037'

def render2(x, y, scan):
	off = 0
	i = 0
	while i < y:
		off = off + 320
		i = i + 1
	off = off + x
	if scan:
		pixel(scan & 0x80, off + 0)
		pixel(scan & 0x40, off + 1)
		pixel(scan & 0x20, off + 2)
		pixel(scan & 0x10, off + 3)
		pixel(scan & 0x08, off + 4)
		pixel(scan & 0x04, off + 5)
		pixel(scan & 0x02, off + 6)
		pixel(scan & 0x01, off + 7)
	else:
		fb[off:off+8] = '\037\037\037\037\037\037\037\037'

def render(x, y, glyph):
	font0	= vga.font0
	off = 0
	i = 0
	while i < glyph:
		off = off + 32
		i = i + 1

	i = 0
	while i < 16:
		render2(x, y + i, ord(font0[off]))
		off = off + 1
		i = i + 1

def write(x, y, str):
	i = 0
	while i < len(str):
		render(x,y, ord(str[i]))
		x = x + 8
		i = i + 1
