#include "Python.h"

static int initialized = 0;

void
Py_Initialize(void)
{
	PRINT(" Initializing ");
	
	LOG("> Py_Initialize\n"); 

	PyInterpreterState *interp;
	PyThreadState *tstate;
	PyObject *mod;
	extern void _Py_ReadyTypes(void);

	if (initialized)
		return;
	initialized = 1;

	interp = PyInterpreterState_New();

	if (interp == NULL)
		Py_FatalError("Py_Initialize: can't make first interpreter");

	tstate = PyThreadState_New(interp);

	if (tstate == NULL)
		Py_FatalError("Py_Initialize: can't make first thread");

	(void) PyThreadState_Swap(tstate);

	_Py_ReadyTypes();

	if (!_PyFrame_Init())
		Py_FatalError("Py_Initialize: can't init frames");

	if (!_PyInt_Init())
		Py_FatalError("Py_Initialize: can't init ints");

	interp->modules = PyDict_New();
	if (interp->modules == NULL)
	  Py_FatalError("Py_Initialize: can't make modules dictionary");
	
	mod = _PyBuiltin_Init();
	if (mod == NULL)
	  Py_FatalError("Py_Initialize: can't initialize __builtin__");
	interp->builtins = PyModule_GetDict(mod);
	Py_INCREF(interp->builtins);
	
	mod = init_isr();
	if (mod == NULL)
	  Py_FatalError("Py_Initialize: can't initialize ISR");
	Py_INCREF(mod);

	LOG("< Py_Initialize\n");
	
}

void
Py_Finalize(void)
{
	LOG("> Py_Finalize\n"); 
	PyInterpreterState *interp;
	PyThreadState *tstate;

	initialized = 0;

	tstate = PyThreadState_Get();
	interp = tstate->interp;

	PyInterpreterState_Clear(interp);

	PyThreadState_Swap(NULL);
	PyInterpreterState_Delete(interp);

	PyFrame_Fini();
	PyInt_Fini();
	LOG("< Py_Finalize\n");
}

extern struct _frozen frozenModules[];

void
Py_Main()
{
        PyImport_FrozenModules = frozenModules;
        Py_Initialize();
        PyImport_ImportFrozenModule("__main__");
        Py_Finalize();
}

void
Py_FatalError(const char *msg)
{
	char x[] = {'F', 'A', 'T', 'A', 'L', ' ', 'E', 'R', 'R', 'O', 'R', ':', ' ', '\0'};
	PRINT(x);
	PRINT(msg);
	while (1) {}
}
