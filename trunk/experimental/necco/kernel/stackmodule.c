#include "Python.h"

typedef struct {
        PyObject_HEAD
        PyObject *b_base;
        void *b_ptr;
        int b_size;
        int b_readonly;
} PyBufferObject;

static PyObject *d = NULL;

static PyObject *
stack_eval(PyObject *self, PyObject *args)
{
	PyObject *obuf, *ofunc;
	PyBufferObject *pbo;

	if(!d)	{
		PyObject *s = PyThreadState_Get()->interp->modules;
		PyObject *m = PyDict_GetItemString(s, "__main__");

		d = PyModule_GetDict(m);
	}

	if(!PyArg_UnpackTuple(args, "eval", 2, 2, &obuf, &ofunc))
		return NULL;

	if(!PyBuffer_Check(obuf))
		return NULL;

	pbo = (PyBufferObject *)obuf;
	if(pbo->b_readonly)
		return NULL;

	return swapandeval(pbo->b_ptr + pbo->b_size,
				PyFunction_GET_CODE(ofunc), d);
}

static PyMethodDef stack_methods[] = {
	{"eval",	stack_eval,     METH_VARARGS, NULL/*doc*/},
	{NULL,		NULL},
};

PyObject *
_Stack_Init(void)
{
	PyObject *mod, *dict;

	return Py_InitModule4("stack", stack_methods,
			     NULL/*doc*/, (PyObject *)NULL,
			     PYTHON_API_VERSION);
}
