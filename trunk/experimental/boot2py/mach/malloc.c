#include "Python.h"

static void *curr = (void *) 0x00100000;

void *
malloc(size_t size)
{
#ifdef VERBOSE_MALLOC
	print("malloc ");
	print_hex(size);
	print(" @ ");
	print_hex(curr);
	print("   ");
#endif
	
	return (void *) (curr += size);
}
