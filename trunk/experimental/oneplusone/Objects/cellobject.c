#include "Python.h"

PyObject *
PyCell_New(PyObject *obj)
{
	PyCellObject *op;

	op = (PyCellObject *)PyObject_GC_New(PyCellObject, &PyCell_Type);
	op->ob_ref = obj;
	Py_XINCREF(obj);

	_PyObject_GC_TRACK(op);
	return (PyObject *)op;
}

PyTypeObject PyCell_Type = {
  PyObject_HEAD_INIT(&PyType_Type)
};
