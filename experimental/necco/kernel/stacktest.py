import buf
import stack

print '-- test stack swapping --'

def foo():
	print 'in new stack'
	return 'back on old'

newstack = buf.bss(1024)
print stack.eval(newstack, foo)

print '-- test done --'
