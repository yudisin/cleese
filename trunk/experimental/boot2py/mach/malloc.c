#include "Python.h"

static void *curr = 0x50000;

void *
malloc(size_t size)
{
	return curr += size;
}
