
/* Built-in functions */

#include "Python.h"

static PyObject *
builtin_inb(PyObject *self, PyObject *args)
{
	PyObject *v = NULL;

	if (!PyArg_UnpackTuple(args, "inb", 1, 1, &v))
		return NULL;
	
	if (!PyInt_CheckExact(v)) {
		/* ERROR */
		return NULL;
	}

	unsigned short port = PyInt_AS_LONG(v);
	unsigned char data = in(port);
	PyObject *result = PyInt_FromLong(data);

	return result;
}

static PyObject *
builtin_raw_input(PyObject *self, PyObject *args)
{
	PyObject *v = NULL;

	if (!PyArg_UnpackTuple(args, "[raw_]input", 0, 1, &v))
		return NULL;

	PyObject *po;
	char *prompt;
	char *s;
	PyObject *result;
	if (v != NULL) {
		po = PyObject_Str(v);
		if (po == NULL)
			return NULL;
		prompt = PyString_AsString(po);
		if (prompt == NULL)
			return NULL;
	}
	else {
		po = NULL;
		prompt = "";
	}

	print(prompt);
	s = "Ni!"; /* TO DO - implement gets */
	
	Py_XDECREF(po);
	if (s == NULL) {
		/* ERROR */
		return NULL;
	}
	if (*s == '\0') {
		/* ERROR */
		result = NULL;
	}
	else { /* strip trailing '\n' */
		size_t len = strlen(s);
		result = PyString_FromStringAndSize(s, (int)(len-1));
	}
	PyMem_FREE(s);
	return result;
}

static PyMethodDef builtin_methods[] = {
 	{"inb",	        builtin_inb,        METH_VARARGS, NULL/*doc*/},
 	{"raw_input",   builtin_raw_input,  METH_VARARGS, NULL/*doc*/},
	{NULL,		NULL},
};

PyObject *
_PyBuiltin_Init(void)
{
	PyObject *mod;
	mod = Py_InitModule4("__builtin__", builtin_methods,
			     NULL/*doc*/, (PyObject *)NULL,
			     PYTHON_API_VERSION);
	return mod;
}
