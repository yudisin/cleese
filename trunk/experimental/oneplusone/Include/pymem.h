#ifndef Py_PYMEM_H
#define Py_PYMEM_H

#define PyMem_MALLOC(n) malloc((n) ? (n) : 1)
#define PyMem_REALLOC(p, n)     realloc((p), (n) ? (n) : 1)

#define PyMem_FREE           	PyObject_FREE

#define PyMem_NEW(type, n) ( (type *) PyMem_MALLOC((n) * sizeof(type)) )

#define PyMem_DEL PyObject_FREE

#endif /* !Py_PYMEM_H */
