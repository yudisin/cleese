#include "Python.h"

PyObject *
PyTuple_New(register int size)
{
	register PyTupleObject *op;
	if (size < 0) {
		/* ERROR */
		printf("PyTuple_New ERROR\n");
		return NULL;
	}
	{ int nbytes = size * sizeof(PyObject *);
	/* Check for overflow */
	if (nbytes / sizeof(PyObject *) != (size_t)size ||
	    (nbytes += sizeof(PyTupleObject) - sizeof(PyObject *))
	    <= 0)
	{
		return NULL; /* NO MEM ERROR */
	}}
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
		printf("PyTuple_Size ERROR\n");
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
		printf("PyTuple_GetItem ERROR 1\n");
		return NULL;
	}
	if (i < 0 || i >= ((PyTupleObject *)op) -> ob_size) {
		/* ERROR */
		printf("PyTuple_GetItem ERROR 2\n");
		return NULL;
	}
	return ((PyTupleObject *)op) -> ob_item[i];
}

int
PyTuple_SetItem(register PyObject *op, register int i, PyObject *newitem)
{
	register PyObject *olditem;
	register PyObject **p;
	if (!PyTuple_Check(op) || op->ob_refcnt != 1) {
		Py_XDECREF(newitem);
		printf("BadInternalCall");
		return -1;
	}
	if (i < 0 || i >= ((PyTupleObject *)op) -> ob_size) {
		Py_XDECREF(newitem);
		printf("tuple assignment index out of range");
		return -1;
	}
	p = ((PyTupleObject *)op) -> ob_item + i;
	olditem = *p;
	*p = newitem;
	Py_XDECREF(olditem);
	return 0;
}

PyTypeObject PyTuple_Type = {
	PyObject_HEAD_INIT(&PyType_Type)
	0,
	"tuple",
	sizeof(PyTupleObject) - sizeof(PyObject *),
	sizeof(PyObject *),
	0, //(destructor)tupledealloc,		/* tp_dealloc */
	0, //(printfunc)tupleprint,			/* tp_print */
	0,					/* tp_getattr */
	0,					/* tp_setattr */
	0,					/* tp_compare */
	0, //(reprfunc)tuplerepr,			/* tp_repr */
	0,					/* tp_as_number */
	0, //&tuple_as_sequence,			/* tp_as_sequence */
	0, //&tuple_as_mapping,			/* tp_as_mapping */
	0, //(hashfunc)tuplehash,			/* tp_hash */
	0,					/* tp_call */
	0,					/* tp_str */
	PyObject_GenericGetAttr,		/* tp_getattro */
	0,					/* tp_setattro */
	0,					/* tp_as_buffer */
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_GC | Py_TPFLAGS_BASETYPE,		/* tp_flags */
	0, //tuple_doc,				/* tp_doc */
 	0, //(traverseproc)tupletraverse,		/* tp_traverse */
	0,					/* tp_clear */
	0, //tuplerichcompare,			/* tp_richcompare */
	0,					/* tp_weaklistoffset */
	0, //tuple_iter,	    			/* tp_iter */
	0,					/* tp_iternext */
	0, //tuple_methods,				/* tp_methods */
	0,					/* tp_members */
	0,					/* tp_getset */
	0,					/* tp_base */
	0,					/* tp_dict */
	0,					/* tp_descr_get */
	0,					/* tp_descr_set */
	0,					/* tp_dictoffset */
	0,					/* tp_init */
	0,					/* tp_alloc */
	0, //tuple_new,				/* tp_new */
	0, //PyObject_GC_Del,        		/* tp_free */
};

/* The following function breaks the notion that tuples are immutable:
   it changes the size of a tuple.  We get away with this only if there
   is only one module referencing the object.  You can also think of it
   as creating a new tuple object and destroying the old one, only more
   efficiently.  In any case, don't use this if the tuple may already be
   known to some other part of the code. */

int
_PyTuple_Resize(PyObject **pv, int newsize)
{
	register PyTupleObject *v;
	register PyTupleObject *sv;
	int i;
	int oldsize;

	v = (PyTupleObject *) *pv;
	if (v == NULL || v->ob_type != &PyTuple_Type ||
	    (v->ob_size != 0 && v->ob_refcnt != 1)) {
		*pv = 0;
		Py_XDECREF(v);
		PyErr_BadInternalCall();
		return -1;
	}
	oldsize = v->ob_size;
	if (oldsize == newsize)
		return 0;

	if (oldsize == 0) {
		/* Empty tuples are often shared, so we should never 
		   resize them in-place even if we do own the only
		   (current) reference */
		Py_DECREF(v);
		*pv = PyTuple_New(newsize);
		return *pv == NULL ? -1 : 0;
	}

	/* XXX UNREF/NEWREF interface should be more symmetrical */
//	_Py_DEC_REFTOTAL;
//      _PyObject_GC_UNTRACK(v);
//      _Py_ForgetReference((PyObject *) v);
	/* DECREF items deleted by shrinkage */
	for (i = newsize; i < oldsize; i++) {
		Py_XDECREF(v->ob_item[i]);
		v->ob_item[i] = NULL;
	}
	sv = PyObject_GC_Resize(PyTupleObject, v, newsize);
	if (sv == NULL) {
		*pv = NULL;
//		PyObject_GC_Del(v);
		return -1;
	}
	_Py_NewReference((PyObject *) sv);
	/* Zero out items added by growing */
	if (newsize > oldsize)
		memset(&sv->ob_item[oldsize], 0,
		       sizeof(*sv->ob_item) * (newsize - oldsize));
	*pv = (PyObject *) sv;
	_PyObject_GC_TRACK(sv);
	return 0;
}
