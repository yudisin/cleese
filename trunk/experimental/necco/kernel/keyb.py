import ports
inb = ports.inb

def more_chars():
    return (inb(0x64) & 0x01)

def get_scancode():
    while not more_chars():
        pass
    return inb(0x60)

def translate_scancode(scancode):
    if (scancode & 0x80): # high bit set (key release)
        return None
    return "?E1234567890-=BTqwertyuiop[]N^asdfghjkl;'`Z\\zxcvbnm,./SXXXXXXXXX"[scancode & 0x3F]
