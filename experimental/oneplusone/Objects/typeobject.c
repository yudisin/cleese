#include "Python.h"

int
PyType_IsSubtype(PyTypeObject *a, PyTypeObject *b)
{
	if (!(a->tp_flags & Py_TPFLAGS_HAVE_CLASS))
		return b == a || b == &PyBaseObject_Type;

	do {
		if (a == b)
			return 1;
		a = a->tp_base;
	} while (a != NULL);
	return b == &PyBaseObject_Type;
}	

PyTypeObject PyType_Type = {
	PyObject_HEAD_INIT(&PyType_Type)
	/* TO DO */
};

PyTypeObject PyBaseObject_Type = {
	PyObject_HEAD_INIT(&PyType_Type)
	/* TO DO */
};

static int
half_compare(PyObject *self, PyObject *other)
{
  /* TO DO */
  return 2;
}

int
_PyObject_SlotCompare(PyObject *self, PyObject *other)
{
	int c;

	if (self->ob_type->tp_compare == _PyObject_SlotCompare) {
		c = half_compare(self, other);
		if (c <= 1)
			return c;
	}
	if (other->ob_type->tp_compare == _PyObject_SlotCompare) {
		c = half_compare(other, self);
		if (c < -1)
			return -2;
		if (c <= 1)
			return -c;
	}
	return (void *)self < (void *)other ? -1 :
		(void *)self > (void *)other ? 1 : 0;
}

int
PyType_Ready(PyTypeObject *type)
{
    /* TO DO */
	return 1;
}    
