
/* Built-in functions */

#include "Python.h"
#include <hal.h>
#include <mach/x86/io.h>

static PyObject *
builtin_enable(PyObject *self, PyObject *args)
{
	PyObject *v = NULL;

	if (!PyArg_UnpackTuple(args, "enable", 1, 1, &v))
		return NULL;
	
	if (!PyInt_CheckExact(v)) {
		/* ERROR */
		return NULL;
	} else {
	  HAL.m_pic->enable(PyInt_AS_LONG(v));
	  PyObject *result = PyInt_FromLong(1);

		return result;
	}
}

static PyObject *
builtin_inportb(PyObject *self, PyObject *args)
{
	PyObject *v = NULL;

	if (!PyArg_UnpackTuple(args, "inportb", 1, 1, &v))
		return NULL;
	
	if (!PyInt_CheckExact(v)) {
		/* ERROR */
		return NULL;
	} else {
		unsigned short port = PyInt_AS_LONG(v);
		unsigned char data = inportb(port);
		PyObject *result = PyInt_FromLong(data);

		return result;
	}
}

static PyObject *
builtin_outportb(PyObject *self, PyObject *args)
{
	PyObject *v = NULL;
	PyObject *d = NULL;

	if (!PyArg_UnpackTuple(args, "outportb", 2, 2, &v, &d))
		return NULL;
	
	if (!PyInt_CheckExact(v) || !PyInt_CheckExact(d)) {
		/* ERROR */
		return NULL;
	} else {
		unsigned short port = PyInt_AS_LONG(v);
		unsigned short data = PyInt_AS_LONG(d);
		outportb(port, data);
		PyObject *result = PyInt_FromLong(data);

		return result;
	}
}

static PyObject *
builtin_raw_input(PyObject *self, PyObject *args)
{
	PyObject *v = NULL;
	PyObject *po;
	char *prompt;
	char *s;
	PyObject *result;

	if (!PyArg_UnpackTuple(args, "[raw_]input", 0, 1, &v))
		return NULL;

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

	PRINT(prompt);
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
 	{"enable_interrupt",	builtin_enable,        METH_VARARGS, NULL/*doc*/},
 	{"outportb",	        builtin_outportb,      METH_VARARGS, NULL/*doc*/},
 	{"inportb",	        builtin_inportb,       METH_VARARGS, NULL/*doc*/},
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
