#include "Python.h"

#include "ctype.h"

double
PyFloat_AsDouble(PyObject *op)
{
	PyNumberMethods *nb;
	PyFloatObject *fo;
	double val;

	if (op && PyFloat_Check(op))
		return PyFloat_AS_DOUBLE((PyFloatObject*) op);

	if (op == NULL) {
		PyErr_BadArgument();
		return -1;
	}

	if ((nb = op->ob_type->tp_as_number) == NULL || nb->nb_float == NULL) {
		PyErr_SetString(PyExc_TypeError, "a float is required");
		return -1;
	}

	fo = (PyFloatObject*) (*nb->nb_float) (op);
	if (fo == NULL)
		return -1;
	if (!PyFloat_Check(fo)) {
		PyErr_SetString(PyExc_TypeError,
				"nb_float should return float object");
		return -1;
	}

	val = PyFloat_AS_DOUBLE(fo);
	Py_DECREF(fo);

	return val;
}

static PyNumberMethods float_as_number = {
  0, //(binaryfunc)float_add, /*nb_add*/
  0, //(binaryfunc)float_sub, /*nb_subtract*/
  0, //(binaryfunc)float_mul, /*nb_multiply*/
  0, //(binaryfunc)float_classic_div, /*nb_divide*/
  0, //(binaryfunc)float_rem, /*nb_remainder*/
  0, //(binaryfunc)float_divmod, /*nb_divmod*/
  0, //(ternaryfunc)float_pow, /*nb_power*/
  0, //(unaryfunc)float_neg, /*nb_negative*/
  0, //(unaryfunc)float_pos, /*nb_positive*/
  0, //(unaryfunc)float_abs, /*nb_absolute*/
  0, //(inquiry)float_nonzero, /*nb_nonzero*/
	0,		/*nb_invert*/
	0,		/*nb_lshift*/
	0,		/*nb_rshift*/
	0,		/*nb_and*/
	0,		/*nb_xor*/
	0,		/*nb_or*/
  0, //(coercion)float_coerce, /*nb_coerce*/
  0, //(unaryfunc)float_int, /*nb_int*/
  0, //(unaryfunc)float_long, /*nb_long*/
  0, //(unaryfunc)float_float, /*nb_float*/
	0,		/* nb_oct */
	0,		/* nb_hex */
	0,		/* nb_inplace_add */
	0,		/* nb_inplace_subtract */
	0,		/* nb_inplace_multiply */
	0,		/* nb_inplace_divide */
	0,		/* nb_inplace_remainder */
	0, 		/* nb_inplace_power */
	0,		/* nb_inplace_lshift */
	0,		/* nb_inplace_rshift */
	0,		/* nb_inplace_and */
	0,		/* nb_inplace_xor */
	0,		/* nb_inplace_or */
  0, //float_floor_div, /* nb_floor_divide */
  0, //float_div,	/* nb_true_divide */
	0,		/* nb_inplace_floor_divide */
	0,		/* nb_inplace_true_divide */
};

PyTypeObject PyFloat_Type = {
	PyObject_HEAD_INIT(&PyType_Type)
	0,
	"float",
	sizeof(PyFloatObject),
	0,
	0, //(destructor)float_dealloc,		/* tp_dealloc */
	0, //(printfunc)float_print, 		/* tp_print */
	0,					/* tp_getattr */
	0,					/* tp_setattr */
	0, //(cmpfunc)float_compare, 		/* tp_compare */
	0, //(reprfunc)float_repr,			/* tp_repr */
	&float_as_number,			/* tp_as_number */
	0,					/* tp_as_sequence */
	0,					/* tp_as_mapping */
	0, //(hashfunc)float_hash,			/* tp_hash */
	0,					/* tp_call */
	0, //(reprfunc)float_str,			/* tp_str */
	0, //PyObject_GenericGetAttr,		/* tp_getattro */
	0,					/* tp_setattro */
	0,					/* tp_as_buffer */
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_CHECKTYPES |
		Py_TPFLAGS_BASETYPE,		/* tp_flags */
	0, //float_doc,				/* tp_doc */
 	0,					/* tp_traverse */
	0,					/* tp_clear */
	0,					/* tp_richcompare */
	0,					/* tp_weaklistoffset */
	0,					/* tp_iter */
	0,					/* tp_iternext */
	0, //float_methods,				/* tp_methods */
	0,					/* tp_members */
	0,					/* tp_getset */
	0,					/* tp_base */
	0,					/* tp_dict */
	0,					/* tp_descr_get */
	0,					/* tp_descr_set */
	0,					/* tp_dictoffset */
	0,					/* tp_init */
	0,					/* tp_alloc */
	0, //float_new,				/* tp_new */
};
