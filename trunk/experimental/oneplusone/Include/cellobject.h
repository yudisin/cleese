#ifndef Py_CELLOBJECT_H
#define Py_CELLOBJECT_H

typedef struct {
	PyObject_HEAD
	PyObject *ob_ref;
} PyCellObject;

PyAPI_DATA(PyTypeObject) PyCell_Type;

PyAPI_FUNC(PyObject *) PyCell_New(PyObject *);

#endif /* !Py_TUPLEOBJECT_H */
