#include "Python.h"

extern struct _frozen frozenModules[];

void
Py_Main()
{
	PyImport_FrozenModules = frozenModules;
	Py_Initialize();
	PyImport_ImportFrozenModule("__main__");
	Py_Finalize();
}

int
main(void)
{
        init_screen();
        print("Cleese");
        Py_Main();
	return 0;
}
