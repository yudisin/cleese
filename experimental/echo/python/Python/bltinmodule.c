
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
	} else {
		unsigned short port = PyInt_AS_LONG(v);
		unsigned char data = in(port);
		PyObject *result = PyInt_FromLong(data);

		return result;
	}
}

static PyObject *
builtin_textbuffer(PyObject *self, PyObject *args)
{
	static PyObject *tb = NULL;

	if(!tb)	{
		tb = PyBuffer_FromReadWriteMemory((void *)0xb8000, 80*25*2);
	}
	Py_INCREF(tb);

	return tb;
}

static PyMethodDef builtin_methods[] = {
 	{"inb",	        builtin_inb,        METH_VARARGS, NULL/*doc*/},
	{"textbuffer",	builtin_textbuffer, METH_VARARGS, NULL/*doc*/},
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