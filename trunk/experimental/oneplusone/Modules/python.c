#include "Python.h"

int
main(void)
{
	printf("> main\n");
	Py_Main();
	printf("< main\n");
	return 0;
}
