import py8042

""" Keyboard decode for PS/2 keyboard scan codes """

more_chars   = py8042.more_chars
get_scancode = py8042.get_scancode

_ctl = 0
_alt = 0
_shift = 0
_break = 0

def break_code(scancode):
    global _break, _shift, _ctl, _alt
    if scancode == 0xF0:
	_break = 1
	return 1
    elif _break == 1:
	if   scancode == 0x11: _alt = 0
	elif scancode == 0x14: _ctl = 0
	elif scancode == 0x12: _shift = 0
	elif scancode == 0x59: _shift = 0
	_break = 0
	return 1
    elif scancode == 0x11: _alt = 1;   return 1
    elif scancode == 0x14: _ctl = 1;   return 1
    elif scancode == 0x12: _shift = 1; return 1
    elif scancode == 0x59: _shift = 1; return 1
    elif scancode == 0x58: _shift = 1; return 1
    else:
	return 0

shifted = "_____________\t~______Q!___ZSAW@__CXDE$#__ VFTR%__NBHGY^___MJU*&__<KIO)(__>?L:P_-__\"_{+____\n}_|__"
noshift = "_____________\t`______q1___zsaw2__cxde43__ vftr5__nbhgy6___mju78__,kio09__./l;p--__'_[=____\n]_\\__"

def translate_scancode(scancode):
    if scancode == 0x71 and _alt == 1 and _ctl == 1:
	py8042.reboot()				# reboot on ctl-alt-del
    if scancode == 0xE0: return None		# FIXME

    if break_code(scancode) == 1:
	return None

    if scancode > 0x60: return None	# FIXME
    if _ctl == 1:	return None	# chr(ord(shifted[scancode]) & 0x1F)
    elif _shift == 1:	return shifted[scancode]
    else:		return noshift[scancode]
