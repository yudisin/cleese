
/* Built-in functions */

#include "Python.h"

static PyObject *
builtin___import__(PyObject *self, PyObject *args)
{
	char *name;
	/* @@@ hack until I get ParseTuple working */
	name = ((PyStringObject *)(((PyTupleObject *)args)->ob_item[0]))->ob_sval;
	
	return PyImport_ImportModuleEx(name);
}

static PyMethodDef builtin_methods[] = {
 	{"__import__",	builtin___import__, METH_VARARGS, NULL/*doc*/},
	{NULL,		NULL},
};

PyObject *
_PyBuiltin_Init(void)
{
	PyObject *mod, *dict;
	mod = Py_InitModule4("__builtin__", builtin_methods,
			     NULL/*doc*/, (PyObject *)NULL,
			     PYTHON_API_VERSION);

	dict = PyModule_GetDict(mod);

	#define SETBUILTIN(NAME, OBJECT) \
	if (PyDict_SetItemString(dict, NAME, (PyObject *)OBJECT) < 0)	\
		return NULL;

	SETBUILTIN("None",		Py_None);

	return mod;
}
