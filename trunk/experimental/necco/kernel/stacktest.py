import buf
import stack

print '-- test stacks'

def foo():
	i = init
	while i:
		print '-- swapped: %d' % i
		i = stack.swap(i - 1)
	stack.swap(None)
	print '** not reached **'

newstack = buf.bss(1024)
stack.init(newstack, foo)

init = 8
i = stack.swap(None, newstack)
while i:
	print '-- swapped back: %d' % i
	i = stack.swap(i - 1, newstack)

print '-- test stacks done'
