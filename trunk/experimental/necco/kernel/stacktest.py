import buf
import stack

print '-- test stacks'

def recur(i):
	if i:
		print '-- swapped: %d' % i
		recur(stack.swap(i - 1))
	else:
		stack.swap(None)

def worker():
	recur(8)
	print '** not reached **'

newstack = buf.bss(0x800)
stack.init(newstack, worker)

i = stack.swap(None, newstack)
while i:
	print '-- swapped back: %d' % i
	i = stack.swap(i - 1, newstack)

print '-- test stacks done'
