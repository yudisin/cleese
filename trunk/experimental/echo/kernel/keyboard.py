print "press a key"
while 1:
    while not (inb(0x64) & 1):
        pass
    print inb(0x60)
