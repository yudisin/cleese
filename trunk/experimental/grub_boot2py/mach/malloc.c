#include "Python.h"

static void *curr = (void *) 0x00200000;

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

	void *prev = curr;
	curr += size;
	return (void *) prev;
}
