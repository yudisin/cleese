################################################################################
import pyvga
import blit

pyvga.exittext()
blit.fill(pyvga.framebuffer,320,0,0,320,200,0)

################################################################################
import isr
import py8042

bufchar = None

def kbd_isr():
	global bufchar
	ch = keyb.translate_scancode(keyb.get_scancode())
	if ch:
		bufchar = ch

def poll_mouse_loop():
	fl = py8042.poll_mouse()
	if fl:
		poll_mouse_loop()

dir = None
def clk_isr():
	global bufchar
	global dir
	blit.fill(pyvga.framebuffer, 320, 312, 0, 8, 8, (isr.ticker & 15) + 16)

	poll_mouse_loop()

	dx = py8042.dx
	dy = py8042.dy
	if   dx > 10:  dir = 'l'
	elif dy > 10:  dir = 'k'
	elif dx < 0:   dir = 'h'
	elif dy < 0:   dir = 'j'
	elif dx == 0 and dy == 0 and dir:
		bufchar = dir
		dir = None
	
	py8042.clear_mouse()

isr.setvec(clk_isr, kbd_isr)

################################################################################
import keyb
import buf

map = buf.bss(400)

#mapi = '     #####               #   #               #   #             ###   ##            #      #          ### # ## #   ###### #   # ## #####  ..# #   .$          ..# ##### ### #@##  ..#     #     #########     #######                                                                                                                                                                        '
mapi = '     #####               #   #               #$  #             ###  $##            #  $ $ #          ### # ## #   ###### #   # ## #####  ..# # $  $          ..# ##### ### #@##  ..#     #     #########     #######                                                                                                                                                                        '
#mapi = '           #######             #  ...#         #####  ...#         #      . .#         #  ##  ...#         ## ##  ...#        ### ########        # $$$ ##        #####  $ $ #####   ##   #$ $   #   #   #@ $  $    $  $ #   ###### $$ $ #####        #      #            ########                                                                                                          '

map[:len(mapi)] = mapi

def where():
	i = 0
	l = len(mapi)
	while i < l:
		ch = ord(map[i])
		if ch == ord('@'):	return i
		if ch == ord('&'):	return i
		i = i + 1
	return -1

def tile(x, y, char):
	x = x << 3
	y = y << 3
	o = ord(char)
	if   o == ord(' '): v = 0
	elif o == ord('#'): v = 31
	elif o == ord('.'): v = 5
	elif o == ord('*'): v = 2
	elif o == ord('$'): v = 4
	elif o == ord('@'): v = 1
	elif o == ord('&'): v = 1
	blit.fill(pyvga.framebuffer, 320, x, y, 8, 8, v)

def disptile(off):
	y = 0
	n = off
	while n > 20:
		n = n + -20
		y = y + 1
	x = n
	tile(x, y, map[off])

def dump(verbose):
	off = 0
	y = 0
	while off < len(mapi):
		x = 0
		while x < 20:
			tile(x, y, map[off])
			off = off + 1
			x = x + 1
		y = y + 1

def do_paint():
	dump(0)

s = buf.bss(3)

def move(dir):
	global s
	soko = where()

	s[0] = map[soko]
	s[1] = map[soko+dir]
	s[2] = map[soko+dir+dir]

	rewrite()

	map[soko]         = s[0]
	map[soko+dir]     = s[1]
	map[soko+dir+dir] = s[2]

	disptile(soko)
	disptile(soko+dir)
	disptile(soko+dir+dir)

def rewrite():
	s1 = ord(s[1])
	if s1 == ord(' ') or s1 == ord('.'):
		s[0] = leave(s[0])
		s[1] = enter(s[1])
	elif s1 == ord('$') or s1 == ord('*'):
		s2 = ord(s[2])
		if s2 == ord(' ') or s2 == ord('.'):
			s[0] = leave(s[0])
			s[1] = enter(s[1])
			s[2] = slide(s[2])

def leave(c):
        if ord(c) == ord('@'): return ' '
        else:        return '.'

def enter(c):
        if ord(c) == ord(' ') or ord(c) == ord('$'): return '@'
        else:                    return '&'

def slide(c):
        if ord(c) == ord(' '): return '$'
        else:        return '*'

def stones():
	i = 0
	l = len(mapi)
	while i < l:
		if ord(map[i]) == ord('$'):
			return 1
		i = i + 1
	return 0

def loop(msg):
	pyvga.cleartext()
	pyvga.entertext()
	print msg
	while 1: pass

do_paint()
done = 0
while not done:
	if not stones():
		loop('You Win!')

	while not bufchar:
		pass
	key = ord(bufchar)
	bufchar = None

	if key == ord('q'):	done = 1
	elif key == ord('h'):	move(-1)
	elif key == ord('j'):	move(20)
	elif key == ord('k'):	move(-20)
	elif key == ord('l'):	move(1)
	elif key == ord('p'):	dump(1)

loop('Quit')
