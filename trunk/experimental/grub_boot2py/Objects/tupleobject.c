#include "Python.h"

PyObject *
PyTuple_New(register int size)
{
	register PyTupleObject *op;
	if (size < 0) {
	  /* ERROR */
		return NULL;
	}
	int nbytes = size * sizeof(PyObject *);
	/* Check for overflow */
	if (nbytes / sizeof(PyObject *) != (size_t)size ||
	    (nbytes += sizeof(PyTupleObject) - sizeof(PyObject *))
	    <= 0)
	{
	  return NULL; /* NO MEM ERROR */
	}
	op = PyObject_GC_NewVar(PyTupleObject, &PyTuple_Type, size);
	if (op == NULL)
		return NULL;
	memset(op->ob_item, 0, sizeof(*op->ob_item) * size);
	_PyObject_GC_TRACK(op);
	return (PyObject *) op;
}

int
PyTuple_Size(register PyObject *op)
{
	if (!PyTuple_Check(op)) {
	  /* ERROR */
		return -1;
	}
	else
		return ((PyTupleObject *)op)->ob_size;
}

PyObject *
PyTuple_GetItem(register PyObject *op, register int i)
{
	if (!PyTuple_Check(op)) {
	  /* ERROR */
		return NULL;
	}
	if (i < 0 || i >= ((PyTupleObject *)op) -> ob_size) {
	  /* ERROR */
		return NULL;
	}
	return ((PyTupleObject *)op) -> ob_item[i];
}

PyTypeObject PyTuple_Type = {
  PyObject_HEAD_INIT(&PyType_Type)
  /* TO DO */
};
