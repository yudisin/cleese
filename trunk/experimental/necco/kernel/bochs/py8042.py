import ports
inb  = ports.inb
outb = ports.outb

### KEYBOARD ###################################################################

def more_chars():
    return (inb(0x64) & 0x01)

def get_scancode():
    while not more_chars():
        pass
    return inb(0x60)

### MOUSE ######################################################################

squeaks = 0

dx = 0
dy = 0

def poll_mouse():
	def mouse_ready():
		return (inb(0x64) & 0x21) == 0x21

	if not mouse_ready():
		return 0

	global squeaks
	squeaks = squeaks + 1

	global dx
	global dy
	fl = inb(0x60)
	while not mouse_ready(): pass
	dx = inb(0x60)
	while not mouse_ready(): pass
	dy = inb(0x60)
	if fl & 0x20: dy = -(256 + -dy)
	if fl & 0x10: dx = -(256 + -dx)
	return fl & 0xCF

def clear_mouse():
	global dx
	global dy
	dx = 0
	dy = 0

### INIT #######################################################################

def wait_for_8042():
	while (inb(0x64) & 0x02):	# AUX_STATUS & IBF
		pass

def swallow_ack():
	while not (inb(0x64) & 0x1): pass
	inb(0x60)

def keyboard_kludge():
	""" [DL] my laptop will reboot if the mouse is used in XT mode """
	wait_for_8042()
	outb(0x60, 0x64)	# SET_FLAGS, AUX_COMMAND
	outb(0x05, 0x60)	# set keyboard to AT mode

def reboot():
	wait_for_8042()	
	outb(0xfe, 0x64)

keyboard_kludge()

wait_for_8042(); outb(0xa8, 0x64)	# AUX_ENABLE, AUX_COMMAND

wait_for_8042(); outb(0xd4, 0x64)	# AUX_WRITE, AUX_COMMAND
wait_for_8042(); outb(0xf4, 0x60)	# AUX_ENABLE_DEV, AUX_OUTPUT_PORT
swallow_ack()
