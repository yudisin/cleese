#include "Python.h"
#include "ctype.h"
#include "structmember.h"
#include "longintrepr.h"

#define NEW_STYLE_NUMBER(o) PyType_HasFeature((o)->ob_type, \
				Py_TPFLAGS_CHECKTYPES)

/* Shorthands to return certain errors */

static PyObject *
type_error(const char *msg)
{
	PyErr_SetString(PyExc_TypeError, msg);
	return NULL;
}

static PyObject *
null_error(void)
{
	if (!PyErr_Occurred())
		PyErr_SetString(PyExc_SystemError,
				"null argument to internal routine");
	return NULL;
}

/* Operations on any object */

int
PyObject_Cmp(PyObject *o1, PyObject *o2, int *result)
{
	int r;

	if (o1 == NULL || o2 == NULL) {
		null_error();
		return -1;
	}
	r = PyObject_Compare(o1, o2);
	if (PyErr_Occurred())
		return -1;
	*result = r;
	return 0;
}

PyObject *
PyObject_Type(PyObject *o)
{
	PyObject *v;

	if (o == NULL)
		return null_error();
	v = (PyObject *)o->ob_type;
	Py_INCREF(v);
	return v;
}

int
PyObject_Size(PyObject *o)
{
	PySequenceMethods *m;

	if (o == NULL) {
		null_error();
		return -1;
	}

	m = o->ob_type->tp_as_sequence;
	if (m && m->sq_length)
		return m->sq_length(o);

	return PyMapping_Size(o);
}


int
PyObject_SetItem(PyObject *o, PyObject *key, PyObject *value)
{
        PyMappingMethods *m;

        if (o == NULL || key == NULL || value == NULL) {
//               null_error();
                return -1;
        }
        m = o->ob_type->tp_as_mapping;
        if (m && m->mp_ass_subscript)
                return m->mp_ass_subscript(o, key, value);

        if (o->ob_type->tp_as_sequence) {
//                if (PyInt_Check(key))
//                        return PySequence_SetItem(o, PyInt_AsLong(key), value);
                        return PySequence_SetItem(o, PyInt_AS_LONG(key), value);
//                else if (PyLong_Check(key)) {
//                        long key_value = PyLong_AsLong(key);
//                        if (key_value == -1 && PyErr_Occurred())
//                                return -1;
//                        return PySequence_SetItem(o, key_value, value);
//                }
//                type_error("sequence index must be integer");
                return -1;
        }

//        type_error("object does not support item assignment");
        return -1;
}


/* Operations on numbers */

int
PyNumber_Check(PyObject *o)
{
	return o && o->ob_type->tp_as_number &&
	       (o->ob_type->tp_as_number->nb_int ||
		o->ob_type->tp_as_number->nb_float);
}

#define NB_SLOT(x) offsetof(PyNumberMethods, x)
#define NB_BINOP(nb_methods, slot) \
		(*(binaryfunc*)(& ((char*)nb_methods)[slot]))


static PyObject *
binary_op1(PyObject *v, PyObject *w, const int op_slot)
{
	PyObject *x;
	binaryfunc slotv = NULL;
	binaryfunc slotw = NULL;

	if (v->ob_type->tp_as_number != NULL && NEW_STYLE_NUMBER(v))
		slotv = NB_BINOP(v->ob_type->tp_as_number, op_slot);
	if (w->ob_type != v->ob_type &&
	    w->ob_type->tp_as_number != NULL && NEW_STYLE_NUMBER(w)) {
		slotw = NB_BINOP(w->ob_type->tp_as_number, op_slot);
		if (slotw == slotv)
			slotw = NULL;
	}
	if (slotv) {
		if (slotw && PyType_IsSubtype(w->ob_type, v->ob_type)) {
			x = slotw(v, w);
			if (x != Py_NotImplemented)
				return x;
			Py_DECREF(x); /* can't do it */
			slotw = NULL;
		}
		x = slotv(v, w);
		if (x != Py_NotImplemented)
			return x;
		Py_DECREF(x); /* can't do it */
	}
	if (slotw) {
		x = slotw(v, w);
		if (x != Py_NotImplemented)
			return x;
		Py_DECREF(x); /* can't do it */
	}

	if (!NEW_STYLE_NUMBER(v) || !NEW_STYLE_NUMBER(w)) {

		int err = PyNumber_CoerceEx(&v, &w);
		if (err < 0) {
			return NULL;
		}
		if (err == 0) {
			PyNumberMethods *mv = v->ob_type->tp_as_number;
			if (mv) {
				binaryfunc slot;
				slot = NB_BINOP(mv, op_slot);
				if (slot) {
					PyObject *x = slot(v, w);
					Py_DECREF(v);
					Py_DECREF(w);
					return x;
				}
			}
			/* CoerceEx incremented the reference counts */
			Py_DECREF(v);
			Py_DECREF(w);
		}
	}
	Py_INCREF(Py_NotImplemented);
	return Py_NotImplemented;
}

static PyObject *
binop_type_error(PyObject *v, PyObject *w, const char *op_name)
{
	/* ERROR */
	printf("binop_type_error\n");
	return NULL;
}

static PyObject *
binary_op(PyObject *v, PyObject *w, const int op_slot, const char *op_name)
{
	PyObject *result = binary_op1(v, w, op_slot);
	if (result == Py_NotImplemented) {
		Py_DECREF(result);
		return binop_type_error(v, w, op_name);
	}
	return result;
}

#define BINARY_FUNC(func, op, op_name) \
    PyObject * \
    func(PyObject *v, PyObject *w) { \
	    return binary_op(v, w, NB_SLOT(op), op_name); \
    }

BINARY_FUNC(PyNumber_And, nb_and, "&")
BINARY_FUNC(PyNumber_Lshift, nb_lshift, "<<")
BINARY_FUNC(PyNumber_Rshift, nb_rshift, ">>")

PyObject *
PyNumber_Remainder(PyObject *v, PyObject *w)
{
	return binary_op(v, w, NB_SLOT(nb_remainder), "%");
}

PyObject *
PyObject_CallObject(PyObject *o, PyObject *a)
{
	return PyEval_CallObjectWithKeywords(o, a, NULL);
}

PyObject *
PyObject_Call(PyObject *func, PyObject *arg, PyObject *kw)
{
        ternaryfunc call;

	if ((call = func->ob_type->tp_call) != NULL) {
		PyObject *result = (*call)(func, arg, kw);
		return result;
	}
	return NULL;
}

PyObject *
PyObject_CallFunction(PyObject *callable, char *format, ...)
{
	va_list va;
	PyObject *args, *retval;

	if (callable == NULL)
		return null_error();

	if (format && *format) {
		va_start(va, format);
		args = Py_VaBuildValue(format, va);
		va_end(va);
	}
	else
		args = PyTuple_New(0);

	if (args == NULL)
		return NULL;

	if (!PyTuple_Check(args)) {
		PyObject *a;

		a = PyTuple_New(1);
		if (a == NULL)
			return NULL;
		if (PyTuple_SetItem(a, 0, args) < 0)
			return NULL;
		args = a;
	}
	retval = PyObject_Call(callable, args, NULL);

	Py_DECREF(args);

	return retval;
}

PyObject *
PyObject_CallMethod(PyObject *o, char *name, char *format, ...)
{
	va_list va;
	PyObject *args, *func = 0, *retval;

	if (o == NULL || name == NULL)
		return null_error();

	func = PyObject_GetAttrString(o, name);
	if (func == NULL) {
		PyErr_SetString(PyExc_AttributeError, name);
		return 0;
	}

	if (!PyCallable_Check(func))
		return type_error("call of non-callable attribute");

	if (format && *format) {
		va_start(va, format);
		args = Py_VaBuildValue(format, va);
		va_end(va);
	}
	else
		args = PyTuple_New(0);

	if (!args)
		return NULL;

	if (!PyTuple_Check(args)) {
		PyObject *a;

		a = PyTuple_New(1);
		if (a == NULL)
			return NULL;
		if (PyTuple_SetItem(a, 0, args) < 0)
			return NULL;
		args = a;
	}

	retval = PyObject_Call(func, args, NULL);

	Py_DECREF(args);
	Py_DECREF(func);

	return retval;
}

int PyObject_AsCharBuffer(PyObject *obj,
			  const char **buffer,
			  int *buffer_len)
{
	PyBufferProcs *pb;
	const char *pp;
	int len;

	if (obj == NULL || buffer == NULL || buffer_len == NULL) {
		null_error();
		return -1;
	}
	pb = obj->ob_type->tp_as_buffer;
	if (pb == NULL ||
	     pb->bf_getcharbuffer == NULL ||
	     pb->bf_getsegcount == NULL) {
		PyErr_SetString(PyExc_TypeError,
				"expected a character buffer object");
		return -1;
	}
	if ((*pb->bf_getsegcount)(obj,NULL) != 1) {
		PyErr_SetString(PyExc_TypeError,
				"expected a single-segment buffer object");
		return -1;
	}
	len = (*pb->bf_getcharbuffer)(obj, 0, &pp);
	if (len < 0)
		return -1;
	*buffer = pp;
	*buffer_len = len;
	return 0;
}

/* Operations on sequences */

int
PySequence_Check(PyObject *s)
{
	return s != NULL && s->ob_type->tp_as_sequence &&
		s->ob_type->tp_as_sequence->sq_item != NULL;
}

int
PySequence_Size(PyObject *s)
{
	PySequenceMethods *m;

	if (s == NULL) {
		null_error();
		return -1;
	}

	m = s->ob_type->tp_as_sequence;
	if (m && m->sq_length)
		return m->sq_length(s);

	type_error("len() of unsized object");
	return -1;
}

PyObject *
PySequence_Tuple(PyObject *v)
{
	PyObject *it;  /* iter(v) */
	int n;         /* guess for result tuple size */
	PyObject *result;
	int j;

	if (v == NULL)
		return null_error();

	/* Special-case the common tuple and list cases, for efficiency. */
	if (PyTuple_CheckExact(v)) {
		/* Note that we can't know whether it's safe to return
		   a tuple *subclass* instance as-is, hence the restriction
		   to exact tuples here.  In contrast, lists always make
		   a copy, so there's no need for exactness below. */
		Py_INCREF(v);
		return v;
	}
	if (PyList_Check(v))
		return PyList_AsTuple(v);

	/* Get iterator. */
	it = PyObject_GetIter(v);
	if (it == NULL)
		return NULL;

	/* Guess result size and allocate space. */
	n = PySequence_Size(v);
	if (n < 0) {
		PyErr_Clear();
		n = 10;  /* arbitrary */
	}
	result = PyTuple_New(n);
	if (result == NULL)
		goto Fail;

	/* Fill the tuple. */
	for (j = 0; ; ++j) {
		PyObject *item = PyIter_Next(it);
		if (item == NULL) {
			if (PyErr_Occurred())
				goto Fail;
			break;
		}
		if (j >= n) {
			if (n < 500)
				n += 10;
			else
				n += 100;
			if (_PyTuple_Resize(&result, n) != 0) {
				Py_DECREF(item);
				goto Fail;
			}
		}
		PyTuple_SET_ITEM(result, j, item);
	}

	/* Cut tuple back if guess was too large. */
	if (j < n &&
	    _PyTuple_Resize(&result, j) != 0)
		goto Fail;

	Py_DECREF(it);
	return result;

Fail:
	Py_XDECREF(result);
	Py_DECREF(it);
	return NULL;
}


int
PySequence_SetItem(PyObject *s, int i, PyObject *o)
{
        PySequenceMethods *m;

        if (s == NULL) {
//                null_error();
                return -1;
        }

        m = s->ob_type->tp_as_sequence;
        if (m && m->sq_ass_item) {
                if (i < 0) {
                        if (m->sq_length) {
                                int l = (*m->sq_length)(s);
                                if (l < 0)
                                        return -1;
                                i += l;
                        }
                }
                return m->sq_ass_item(s, i, o);
        }

//        type_error("object doesn't support item assignment");
        return -1;
}

PyObject *
PySequence_GetItem(PyObject *s, int i)
{
	PySequenceMethods *m;

	if (s == NULL)
		return null_error();

	m = s->ob_type->tp_as_sequence;
	if (m && m->sq_item) {
		if (i < 0) {
			if (m->sq_length) {
				int l = (*m->sq_length)(s);
				if (l < 0)
					return NULL;
				i += l;
			}
		}
		return m->sq_item(s, i);
	}

	return type_error("unindexable object");
}

PyObject *
PySequence_GetSlice(PyObject *s, int i1, int i2)
{
        PySequenceMethods *m;
//        PyMappingMethods *mp;

        if (!s) return null_error();

        m = s->ob_type->tp_as_sequence;
        if (m && m->sq_slice) {
                if (i1 < 0 || i2 < 0) {
                        if (m->sq_length) {
                                int l = (*m->sq_length)(s);
                                if (l < 0)
                                        return NULL;
                                if (i1 < 0)
                                        i1 += l;
                                if (i2 < 0)
                                        i2 += l;
                        }
                }
                return m->sq_slice(s, i1, i2);
//        } else if ((mp = s->ob_type->tp_as_mapping) && mp->mp_subscript) {
//                PyObject *res;
//                PyObject *slice = sliceobj_from_intint(i1, i2);
//                if (!slice)
//                        return NULL;
//                res = mp->mp_subscript(s, slice);
//                Py_DECREF(slice);
//                return res;
        }

        return type_error("unsliceable object");
}

int
PySequence_SetSlice(PyObject *s, int i1, int i2, PyObject *o)
{
        PySequenceMethods *m;
//        PyMappingMethods *mp;

        if (s == NULL) {
//                null_error();
                return -1;
        }

        m = s->ob_type->tp_as_sequence;
        if (m && m->sq_ass_slice) {
                if (i1 < 0 || i2 < 0) {
                        if (m->sq_length) {
                                int l = (*m->sq_length)(s);
                                if (l < 0)
                                        return -1;
                                if (i1 < 0)
                                        i1 += l;
                                if (i2 < 0)
                                        i2 += l;
                        }
                }
                return m->sq_ass_slice(s, i1, i2, o);
//        } else if ((mp = s->ob_type->tp_as_mapping) && mp->mp_ass_subscript) {
//               int res;
//                PyObject *slice = sliceobj_from_intint(i1, i2);
//              if (!slice)
//                      return -1;
//              res = mp->mp_ass_subscript(s, slice, o);
//              Py_DECREF(slice);
//              return res;
        }

//        type_error("object doesn't support slice assignment");
        return -1;
}

PyObject *
PySequence_Fast(PyObject *v, const char *m)
{
	if (v == NULL)
		return null_error();

	if (PyList_CheckExact(v) || PyTuple_CheckExact(v)) {
		Py_INCREF(v);
		return v;
	}

	v = PySequence_Tuple(v);
	if (v == NULL && PyErr_ExceptionMatches(PyExc_TypeError))
		return type_error(m);

	return v;
}

int
PyMapping_Size(PyObject *o)
{
	PyMappingMethods *m;

	if (o == NULL) {
		null_error();
		return -1;
	}

	m = o->ob_type->tp_as_mapping;
	if (m && m->mp_length)
		return m->mp_length(o);

	type_error("len() of unsized object");
	return -1;
}

PyObject *
PyObject_GetItem(PyObject *o, PyObject *key)
{
	PyMappingMethods *m;
//	PySequenceMethods *s;

	if (o == NULL || key == NULL)
		Py_FatalError("null_error");

	m = o->ob_type->tp_as_mapping;
	if (m && m->mp_subscript) {
		return m->mp_subscript(o, key);
	}

        if (o->ob_type->tp_as_sequence) {
		return PySequence_GetItem(o, PyInt_AS_LONG(key));
//                if (PyInt_Check(key))
//                        return PySequence_GetItem(o, PyInt_AsLong(key));
//                else if (PyLong_Check(key)) {
//                       long key_value = PyLong_AsLong(key);
//                        if (key_value == -1 && PyErr_Occurred())
//                                return NULL;
//                        return PySequence_GetItem(o, key_value);
//                }
//                return type_error("sequence index must be integer");
//		return NULL;
        }

	return type_error("unsubscriptable object");
}

PyObject *
PyObject_GetIter(PyObject *o)
{
	PyTypeObject *t = o->ob_type;
	getiterfunc f = NULL;
	if (PyType_HasFeature(t, Py_TPFLAGS_HAVE_ITER))
		f = t->tp_iter;
	if (f == NULL) {
		if (PySequence_Check(o))
			return PySeqIter_New(o);
		PyErr_SetString(PyExc_TypeError,
				"iteration over non-sequence");
		return NULL;
	}
	else {
		PyObject *res = (*f)(o);
		if (res != NULL && !PyIter_Check(res)) {
			PyErr_Format(PyExc_TypeError,
				     "iter() returned non-iterator "
				     "of type '%.100s'",
				     res->ob_type->tp_name);
			Py_DECREF(res);
			res = NULL;
		}
		return res;
	}
}

/* Return next item.
 * If an error occurs, return NULL.  PyErr_Occurred() will be true.
 * If the iteration terminates normally, return NULL and clear the
 * PyExc_StopIteration exception (if it was set).  PyErr_Occurred()
 * will be false.
 * Else return the next object.  PyErr_Occurred() will be false.
 */
PyObject *
PyIter_Next(PyObject *iter)
{
	PyObject *result;
	assert(PyIter_Check(iter));
	result = (*iter->ob_type->tp_iternext)(iter);
	if (result == NULL &&
	    PyErr_Occurred() &&
	    PyErr_ExceptionMatches(PyExc_StopIteration))
		PyErr_Clear();
	return result;
}
