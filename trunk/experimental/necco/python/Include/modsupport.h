
#ifndef Py_MODSUPPORT_H
#define Py_MODSUPPORT_H

/* Module support interface */

PyAPI_FUNC(int) PyArg_Parse(PyObject *, char *, ...);
PyAPI_FUNC(int) PyArg_ParseTuple(PyObject *, char *, ...);
PyAPI_FUNC(int) PyArg_ParseTupleAndKeywords(PyObject *, PyObject *,
                                                  char *, char **, ...);

PyAPI_FUNC(int) PyArg_UnpackTuple(PyObject *, char *, int, int, ...);
PyAPI_FUNC(PyObject *) Py_BuildValue(char *, ...);

#define PYTHON_API_VERSION 1012

PyAPI_FUNC(PyObject *) Py_InitModule4(char *name, PyMethodDef *methods,
                                            char *doc, PyObject *self,
                                            int apiver);

#define Py_InitModule(name, methods) \
	Py_InitModule4(name, methods, (char *)NULL, (PyObject *)NULL, \
		       PYTHON_API_VERSION)

#endif /* !Py_MODSUPPORT_H */
