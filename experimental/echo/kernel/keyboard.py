
def get_scancode():
    while not (inb(0x64) & 1):
        pass
    return inb(0x60)

print "press a key"

while 1:
    print get_scancode()

