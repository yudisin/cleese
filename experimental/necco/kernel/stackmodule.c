#include "Python.h"

typedef struct {
        PyObject_HEAD
        PyObject *b_base;
        void *b_ptr;
        int b_size;
        int b_readonly;
} PyBufferObject;

static PyObject *d = NULL;

static void init_dict() {
	PyObject *s = PyThreadState_Get()->interp->modules;
	PyObject *m = PyDict_GetItemString(s, "__main__");
	d = PyModule_GetDict(m);
}


extern PyObject *swapstacks();
extern int initstack();

static void *save_area(PyObject *stack)
{
	static void *initspsav;
	return (stack ? ((PyBufferObject *)stack)->b_ptr
		      : (void *)&initspsav);
}

static PyObject *
stack_swap(PyObject *self, PyObject *args)
{
	static PyObject *curstack;
	PyObject *obuf, *oarg;
	void *from, *to;

	obuf = NULL;
	if(!PyArg_UnpackTuple(args, "swap", 1, 2, &oarg, &obuf))
		return NULL;

	if(!PyBuffer_Check(obuf) && obuf)
		return NULL;

	from = save_area(curstack);
	to   = save_area(obuf);
	curstack = obuf;

	return swapstacks(from, to, oarg);
}


static PyObject *
stack_init(PyObject *self, PyObject *args)
{
	PyObject *obuf, *ofunc;
	PyBufferObject *pbo;
	extern PyObject *PyEval_EvalCode();

	if(!d)	{ init_dict(); }

	if(!PyArg_UnpackTuple(args, "init", 2, 2, &obuf, &ofunc))
		return NULL;

	if(!PyBuffer_Check(obuf))
		return NULL;

	pbo = (PyBufferObject *)obuf;
	if(pbo->b_readonly)
		return NULL;

	*(int *)save_area(obuf) = initstack(
			pbo->b_ptr + pbo->b_size,
			PyFunction_GET_CODE(ofunc), d);

	Py_INCREF(Py_True);
	return Py_True;
}

static PyMethodDef stack_methods[] = {
	{"swap",	stack_swap,     METH_VARARGS, NULL/*doc*/},
	{"init",	stack_init,	METH_VARARGS, NULL/*doc*/},
	{NULL,		NULL},
};

PyObject *
_Stack_Init(void)
{
	return Py_InitModule4("stack", stack_methods,
			     NULL/*doc*/, (PyObject *)NULL,
			     PYTHON_API_VERSION);
}
