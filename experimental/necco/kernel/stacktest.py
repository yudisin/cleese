import buf
import stack
import isr

isr.setvec()

print '-- test stacks'

def recurse(i):
	if i:
		print '-- swapped: %d' % i
		recurse(stack.swap(i - 1))
	else:
		stack.swap(None)

def worker():
	recurse(8)

newstack = buf.bss(0x800)
stack.init(newstack, worker)

i = stack.swap(None, newstack)
while i:
	print '-- swapped back: %d' % i
	i = stack.swap(i - 1, newstack)

print '-- test stacks done'
print
print '-- test linux emulation'

# this hi.c can be built once, then run on either Linux or Cleese
# -----------------------------------------------------------------------------
#void _start() 
#{
#	char *msg = "Hello, world!\n";
#       write(1, msg, strlen(msg));
#	_exit(0);
#}
# -----------------------------------------------------------------------------
# % gcc -c hi.c
# % ld -static -Ttext 0x80000 hi.o -lc
# for Linux: ./a.out, but for Cleese:
# % objcopy -R .note -R .comment -S -O binary a.out a.bin
# % od -An -tx1 a.bin|tr " \n" "\n "|tail +2|sed -e 's/^/\\x/'|tr -d " \n"

obj ="\x55\x89\xe5\x83\xec\x04\xc7\x45\xfc\x7c\x01\x08\x00\x8b\x45\xfc\x50\xe8\x2a\x00\x00\x00\x83\xc4\x04\x89\xc0\x50\x8b\x45\xfc\x50\x6a\x01\xe8\xf9\x00\x00\x00\x83\xc4\x0c\x6a\x00\xe8\xcf\x00\x00\x00\x83\xc4\x04\xc9\xc3\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x8b\x44\x24\x04\xba\x03\x00\x00\x00\x21\xc2\x74\x24\x7a\x17\x38\x30\x0f\x84\x9f\x00\x00\x00\x40\x38\x30\x0f\x84\x96\x00\x00\x00\x40\x83\xf2\x02\x74\x0b\x38\x30\x0f\x84\x88\x00\x00\x00\x40\x31\xd2\x8b\x08\x83\xc0\x04\x29\xca\x81\xc1\xff\xfe\xfe\xfe\x4a\x73\x58\x31\xca\x81\xe2\x00\x01\x01\x01\x75\x4e\x8b\x08\x83\xc0\x04\x29\xca\x81\xc1\xff\xfe\xfe\xfe\x4a\x73\x3e\x31\xca\x81\xe2\x00\x01\x01\x01\x75\x34\x8b\x08\x83\xc0\x04\x29\xca\x81\xc1\xff\xfe\xfe\xfe\x4a\x73\x24\x31\xca\x81\xe2\x00\x01\x01\x01\x75\x1a\x8b\x08\x83\xc0\x04\x29\xca\x81\xc1\xff\xfe\xfe\xfe\x4a\x73\x0a\x31\xca\x81\xe2\x00\x01\x01\x01\x74\x98\x83\xe8\x04\x81\xe9\xff\xfe\xfe\xfe\x80\xf9\x00\x74\x0f\x40\x84\xed\x74\x0a\xc1\xe9\x10\x40\x80\xf9\x00\x74\x01\x40\x2b\x44\x24\x04\xc3\x90\x90\x90\x90\x90\x89\xda\x8b\x5c\x24\x04\xb8\x01\x00\x00\x00\xcd\x80\x89\xd3\x3d\x01\xf0\xff\xff\x0f\x83\x36\x00\x00\x00\x90\x90\x90\x90\x90\x90\x53\x8b\x54\x24\x10\x8b\x4c\x24\x0c\x8b\x5c\x24\x08\xb8\x04\x00\x00\x00\xcd\x80\x5b\x3d\x01\xf0\xff\xff\x0f\x83\x10\x00\x00\x00\xc3\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\xf7\xd8\xa3\x8c\x11\x08\x00\x50\xe8\x13\x00\x00\x00\x59\x89\x08\xb8\xff\xff\xff\xff\xc3\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x55\x89\xe5\xb8\x8c\x11\x08\x00\x89\xec\x5d\xc3\x48\x65\x6c\x6c\x6f\x2c\x20\x77\x6f\x72\x6c\x64\x21\x0a\x00"

#void _start() 
#{
#        char *s;
#        for(s = "Hello, world!\n"; *s; ++s)     {
#                write(1, s, 1);
#        }
#        _exit(0);
#}
#obj = "\x55\x89\xe5\x83\xec\x04\x90\xc7\x45\xfc\xbc\x00\x08\x00\x8b\x45\xfc\x80\x38\x00\x75\x02\xeb\x18\x6a\x01\x8b\x45\xfc\x50\x6a\x01\xe8\x3b\x00\x00\x00\x83\xc4\x0c\xff\x45\xfc\xeb\xe1\x8d\x76\x00\x6a\x00\xe8\x09\x00\x00\x00\x83\xc4\x04\x89\xf6\xc9\xc3\x90\x90\x89\xda\x8b\x5c\x24\x04\xb8\x01\x00\x00\x00\xcd\x80\x89\xd3\x3d\x01\xf0\xff\xff\x0f\x83\x36\x00\x00\x00\x90\x90\x90\x90\x90\x90\x53\x8b\x54\x24\x10\x8b\x4c\x24\x0c\x8b\x5c\x24\x08\xb8\x04\x00\x00\x00\xcd\x80\x5b\x3d\x01\xf0\xff\xff\x0f\x83\x10\x00\x00\x00\xc3\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\xf7\xd8\xa3\xcc\x10\x08\x00\x50\xe8\x13\x00\x00\x00\x59\x89\x08\xb8\xff\xff\xff\xff\xc3\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x55\x89\xe5\xb8\xcc\x10\x08\x00\x89\xec\x5d\xc3\x48\x65\x6c\x6c\x6f\x2c\x20\x77\x6f\x72\x6c\x64\x21\x0a\x00"

linuxbuf = buf.abs(0x80000, 0x10000)	# would eventually be 0x8048000
linuxbuf[:len(obj)] = obj		# and this would be an ELF loader

stack.linux(linuxbuf)

while 1:
	esp = stack.swap(None, linuxbuf)

	eax = stack.linaddr(linuxbuf, esp + 0x1c)
	ecx = stack.linaddr(linuxbuf, esp + 0x18)
	edx = stack.linaddr(linuxbuf, esp + 0x14)

	if eax == 1:	# exit
		break
	elif eax == 4:	# write
		while edx:
			print "%c" % (stack.linaddr(linuxbuf,ecx) & 0xff)
			ecx = ecx + 1
			edx = edx - 1

print '-- test linux emulation done'

# this is the boa 'hello, world' linux executable:
#obj = "\xe8\x0d\x00\x00\x00\xbb\x00\x00\x00\x00\xb8\x01\x00\x00\x00\xcd\x80\xc3\xbe\x46\x00\x08\x00\xeb\x05\xe8\x0c\x00\x00\x00\xe8\x03\x00\x00\x00\x75\xf4\xc3\xac\x85\xc0\xc3\xa3\x5c\x10\x08\x00\xba\x01\x00\x00\x00\xb9\x5c\x10\x08\x00\xbb\x01\x00\x00\x00\xb8\x04\x00\x00\x00\xcd\x80\xc3\x48\x65\x6c\x6c\x6f\x2c\x20\x77\x6f\x72\x6c\x64\x21\x0a\x00"
