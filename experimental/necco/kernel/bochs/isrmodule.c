#include "Python.h"

static PyObject *d, *c, *k = 0;

int python_isr(void *arg)
{
	PyObject *v = NULL;

	switch((int)arg)	{
	case 0: v = c; break;
	case 1: v = k; break;
	}
	if(v)	{
        	Py_DECREF((PyObject *)PyEval_EvalCode(
		 		PyFunction_GET_CODE(v), d, d));
	}
	return 0;
}

/* eventually this should take arguments.  A dict, perhaps? */
static PyObject *
isr_setvec(PyObject *self, PyObject *args)
{
	PyObject *s = PyThreadState_Get()->interp->modules;
	PyObject *m = PyDict_GetItemString(s, "__main__");

	d = PyModule_GetDict(m);
	c = PyDict_GetItemString(d, "clk_isr");
	k = PyDict_GetItemString(d, "kbd_isr");

	initirqs();

	Py_INCREF(Py_True);
	return Py_True;
}

static PyMethodDef isr_methods[] = {
	{"setvec",	isr_setvec,     METH_VARARGS, NULL/*doc*/},
	{NULL,		NULL},
};

PyObject *
_ISR_Init(void)
{
	return Py_InitModule4("isr", isr_methods,
			     NULL/*doc*/, (PyObject *)NULL,
			     PYTHON_API_VERSION);
}
