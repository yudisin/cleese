#include "Python.h"

typedef struct {
	PyObject_HEAD
	PyObject *md_dict;
} PyModuleObject;

PyObject *
PyModule_New(char *name)
{
	PyModuleObject *m;
	PyObject *nameobj;
	m = PyObject_GC_New(PyModuleObject, &PyModule_Type);
	if (m == NULL)
		return NULL;
	nameobj = PyString_FromString(name);
	m->md_dict = PyDict_New();
	if (m->md_dict == NULL || nameobj == NULL)
		goto fail;
	if (PyDict_SetItemString(m->md_dict, "__name__", nameobj) != 0)
		goto fail;
	if (PyDict_SetItemString(m->md_dict, "__doc__", Py_None) != 0)
		goto fail;
	Py_DECREF(nameobj);
	PyObject_GC_Track(m);
	return (PyObject *)m;

 fail:
	Py_XDECREF(nameobj);
	Py_DECREF(m);
	return NULL;
}

PyObject *
PyModule_GetDict(PyObject *m)
{
	PyObject *d;
	if (!PyModule_Check(m)) {
	  /* ERROR */
		return NULL;
	}
	d = ((PyModuleObject *)m) -> md_dict;
	if (d == NULL)
		((PyModuleObject *)m) -> md_dict = d = PyDict_New();
	return d;
}

PyTypeObject PyModule_Type = {
	PyObject_HEAD_INIT(&PyType_Type)
	/* TO DO */
};

