
/* Method object interface */

#ifndef Py_METHODOBJECT_H
#define Py_METHODOBJECT_H

PyAPI_DATA(PyTypeObject) PyCFunction_Type;

typedef PyObject *(*PyCFunction)(PyObject *, PyObject *);

struct PyMethodDef {
	char	     *ml_name;
	PyCFunction  ml_meth;
	int	      ml_flags;
	char	      *ml_doc;
};

typedef struct PyMethodDef PyMethodDef;

PyAPI_FUNC(PyObject *) PyCFunction_NewEx(PyMethodDef *, PyObject *, 
					 PyObject *);

/* Flag passed to newmethodobject */
#define METH_VARARGS  0x0001

typedef struct {
	PyObject_HEAD
	PyMethodDef *m_ml;
	PyObject    *m_self;
	PyObject    *m_module;
} PyCFunctionObject;

#endif /* !Py_METHODOBJECT_H */
