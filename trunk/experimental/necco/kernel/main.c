#include "Python.h"

extern struct _frozen frozenModules[];
extern void Cleese_Initialize(), Cleese_Finalize();

void
Py_Main()
{
	PyImport_FrozenModules = frozenModules;
	Py_Initialize();
	Cleese_Initialize();
	PyImport_ImportFrozenModule("__main__");
	Cleese_Finalize();
	Py_Finalize();
}

int
main(void)
{
        Py_Main();
	return 0;
}
