#include "Python.h"

int
main(void)
{
	LOG("> main\n");
	Py_Main();
	LOG("< main\n");
	return 0;
}
