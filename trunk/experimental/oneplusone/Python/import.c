#include "Python.h"

#include "marshal.h"
#include "compile.h"
#include "eval.h"

static struct _frozen *
find_frozen(char *name)
{
	printf("> find_frozen\n");
	struct _frozen *p;

	for (p = PyImport_FrozenModules; ; p++) {
		if (p->name == NULL) {
			printf("< find_frozen NULL\n");
			return NULL;
		}
		if (strcmp(p->name, name) == 0)
			break;
	}

	printf("< find_frozen\n");
	return p;
}

PyObject *
PyImport_GetModuleDict(void)
{
  printf("> PyImport_GetModuleDict\n");
	PyInterpreterState *interp = PyThreadState_Get()->interp;
	if (interp->modules == NULL)
		Py_FatalError("PyImport_GetModuleDict: no module dictionary!");
  printf("< PyImport_GetModuleDict\n");
	return interp->modules;
}

int
PyImport_ImportFrozenModule(char *name)
{
	printf("> PyImport_ImportFrozenModule\n");
	struct _frozen *p = find_frozen(name);
	PyObject *co;
	PyObject *m;
	int size;

	if (p == NULL)
		return 0;

	co = PyMarshal_ReadObjectFromString((char *)p->code, size);

	m = PyImport_ExecCodeModule(name, co);
	Py_DECREF(co);

	Py_DECREF(m);

	printf("< PyImport_ImportFrozenModule\n");
	return 1;
}

PyObject *
PyImport_AddModule(char *name)
{
  printf("> PyImport_AddModule\n");
	PyObject *modules = PyImport_GetModuleDict();
	PyObject *m;

	if ((m = PyDict_GetItemString(modules, name)) != NULL &&
	    PyModule_Check(m))
		return m;
	m = PyModule_New(name);
	if (m == NULL)
		return NULL;
	if (PyDict_SetItemString(modules, name, m) != 0) {
		Py_DECREF(m);
		return NULL;
	}
	Py_DECREF(m);
	printf("< PyImport_AddModule\n");
	return m;
}

PyObject *
PyImport_ExecCodeModule(char *name, PyObject *co)
{
	printf("> PyImport_ExecCodeModule\n");

	PyObject *modules = PyImport_GetModuleDict();
	PyObject *m, *d, *v;

	m = PyImport_AddModule(name);

	if (m == NULL)
		return NULL;
	d = PyModule_GetDict(m);

	v = PyEval_EvalCode((PyCodeObject *)co, d, d);

	if (v == NULL)
		return NULL;
	Py_DECREF(v);

	if ((m = PyDict_GetItemString(modules, name)) == NULL) {
		/* TO DO: flag error */
		return NULL;
	}

	Py_INCREF(m);

	printf("< PyImport_ExecCodeModule\n");
	return m;
}