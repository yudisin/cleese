#include "Python.h"

PyObject *
PyObject_Init(PyObject *op, PyTypeObject *tp)
{
  printf("> PyObject_Init\n");
	if (op == NULL)
	  Py_FatalError("out of memory");
	/* Any changes should be reflected in PyObject_INIT (objimpl.h) */
	op->ob_type = tp;
	_Py_NewReference(op);
	printf("< PyObject_Init\n");
	return op;
}

#define RICHCOMPARE(t) (PyType_HasFeature((t), Py_TPFLAGS_HAVE_RICHCOMPARE) \
                         ? (t)->tp_richcompare : NULL)

static int swapped_op[] = {Py_GT, Py_GE, Py_EQ, Py_NE, Py_LT, Py_LE};

static PyObject *
try_rich_compare(PyObject *v, PyObject *w, int op)
{
        richcmpfunc f;
        PyObject *res;

        if (v->ob_type != w->ob_type &&
            PyType_IsSubtype(w->ob_type, v->ob_type) &&
            (f = RICHCOMPARE(w->ob_type)) != NULL) {
                res = (*f)(w, v, swapped_op[op]);
                if (res != Py_NotImplemented)
                        return res;
                Py_DECREF(res);
        }
        if ((f = RICHCOMPARE(v->ob_type)) != NULL) {
                res = (*f)(v, w, op);
                if (res != Py_NotImplemented)
                        return res;
                Py_DECREF(res);
        }
        if ((f = RICHCOMPARE(w->ob_type)) != NULL) {
                return (*f)(w, v, swapped_op[op]);
        }
        res = Py_NotImplemented;
        Py_INCREF(res);
        return res;
}

/* try_rich_compare_bool */

/* try_rich_to_3way_compare */

static int
try_3way_compare(PyObject *v, PyObject *w)
{
        int c;
        cmpfunc f;

        /* Comparisons involving instances are given to instance_compare,
           which has the same return conventions as this function. */

        f = v->ob_type->tp_compare;
        if (PyInstance_Check(v))
                return (*f)(v, w);
        if (PyInstance_Check(w))
                return (*w->ob_type->tp_compare)(v, w);

        /* If both have the same (non-NULL) tp_compare, use it. */
        if (f != NULL && f == w->ob_type->tp_compare) {
                c = (*f)(v, w);
                return c;
        }

        /* If either tp_compare is _PyObject_SlotCompare, that's safe. */
        if (f == _PyObject_SlotCompare ||
            w->ob_type->tp_compare == _PyObject_SlotCompare)
                return _PyObject_SlotCompare(v, w);

        /* Try coercion; if it fails, give up */
        c = PyNumber_CoerceEx(&v, &w);
        if (c < 0)
                return -2;
        if (c > 0)
                return 2;

        /* Try v's comparison, if defined */
        if ((f = v->ob_type->tp_compare) != NULL) {
                c = (*f)(v, w);
                Py_DECREF(v);
                Py_DECREF(w);
                return c;
        }

        /* Try w's comparison, if defined */
        if ((f = w->ob_type->tp_compare) != NULL) {
                c = (*f)(w, v); /* swapped! */
                Py_DECREF(v);
                Py_DECREF(w);
                c = c;
                if (c >= -1)
                        return -c; /* Swapped! */
                else
                        return c;
        }

        /* No comparison defined */
        Py_DECREF(v);
        Py_DECREF(w);
        return 2;
}


static int
default_3way_compare(PyObject *v, PyObject *w)
{
        int c;
        char *vname, *wname;

        if (v->ob_type == w->ob_type) {
                /* When comparing these pointers, they must be cast to
                 * integer types (i.e. Py_uintptr_t, our spelling of C9X's
                 * uintptr_t).  ANSI specifies that pointer compares other
                 * than == and != to non-related structures are undefined.
                 */
                Py_uintptr_t vv = (Py_uintptr_t)v;
                Py_uintptr_t ww = (Py_uintptr_t)w;
                return (vv < ww) ? -1 : (vv > ww) ? 1 : 0;
        }

        /* None is smaller than anything */
        if (v == Py_None)
                return -1;
        if (w == Py_None)
                return 1;

        /* different type: compare type names; numbers are smaller */
        if (PyNumber_Check(v))
                vname = "";
        else
                vname = v->ob_type->tp_name;
        if (PyNumber_Check(w))
                wname = "";
        else
                wname = w->ob_type->tp_name;
        c = strcmp(vname, wname);
        if (c < 0)
                return -1;
        if (c > 0)
                return 1;
        /* Same type name, or (more likely) incomparable numeric types */
        return ((Py_uintptr_t)(v->ob_type) < (
                Py_uintptr_t)(w->ob_type)) ? -1 : 1;
}

/* do_cmp */

#define NESTING_LIMIT 20

static int compare_nesting = 0;

static PyObject*
get_inprogress_dict(void)
{
	static PyObject *key;
	PyObject *tstate_dict, *inprogress;

	if (key == NULL) {
		key = PyString_InternFromString("cmp_state");
		if (key == NULL)
			return NULL;
	}

	tstate_dict = PyThreadState_GetDict();
	if (tstate_dict == NULL) {
		/* ERROR */
		return NULL;
	}

	inprogress = PyDict_GetItem(tstate_dict, key);
	if (inprogress == NULL) {
		inprogress = PyDict_New();
		if (inprogress == NULL)
			return NULL;
		if (PyDict_SetItem(tstate_dict, key, inprogress) == -1) {
			Py_DECREF(inprogress);
			return NULL;
		}
		Py_DECREF(inprogress);
	}

	return inprogress;
}

static PyObject *
check_recursion(PyObject *v, PyObject *w, int op)
{
	PyObject *inprogress;
	PyObject *token;
	Py_uintptr_t iv = (Py_uintptr_t)v;
	Py_uintptr_t iw = (Py_uintptr_t)w;
	PyObject *x, *y, *z;

	inprogress = get_inprogress_dict();
	if (inprogress == NULL)
		return NULL;

	token = PyTuple_New(3);
	if (token == NULL)
		return NULL;

	if (iv <= iw) {
                PyTuple_SET_ITEM(token, 0, x = PyLong_FromVoidPtr((void *)v));
                PyTuple_SET_ITEM(token, 1, y = PyLong_FromVoidPtr((void *)w));
                if (op >= 0)
                        op = swapped_op[op];
        } else {
                PyTuple_SET_ITEM(token, 0, x = PyLong_FromVoidPtr((void *)w));
                PyTuple_SET_ITEM(token, 1, y = PyLong_FromVoidPtr((void *)v));
        }
        PyTuple_SET_ITEM(token, 2, z = PyInt_FromLong((long)op));
        if (x == NULL || y == NULL || z == NULL) {
                Py_DECREF(token);
                return NULL;
        }

        if (PyDict_GetItem(inprogress, token) != NULL) {
                Py_DECREF(token);
                return Py_None; /* Without INCREF! */
        }

        if (PyDict_SetItem(inprogress, token, token) < 0) {
                Py_DECREF(token);
                return NULL;
        }

        return token;
}

static void
delete_token(PyObject *token)
{
	PyObject *inprogress;

	if (token == NULL || token == Py_None)
		return;
	inprogress = get_inprogress_dict();
	if (inprogress == NULL)
	  ; /* ERROR */
	else
		PyDict_DelItem(inprogress, token);
	Py_DECREF(token);
}

/* ... */

/* Return (new reference to) Py_True or Py_False. */
static PyObject *
convert_3way_to_object(int op, int c)
{
	PyObject *result;
	switch (op) {
	case Py_LT: c = c <  0; break;
	case Py_LE: c = c <= 0; break;
	case Py_EQ: c = c == 0; break;
	case Py_NE: c = c != 0; break;
	case Py_GT: c = c >  0; break;
	case Py_GE: c = c >= 0; break;
	}
	result = c ? Py_True : Py_False;
	Py_INCREF(result);
	return result;
}

static PyObject *
try_3way_to_rich_compare(PyObject *v, PyObject *w, int op)
{
	int c;

	c = try_3way_compare(v, w);
	if (c >= 2)
		c = default_3way_compare(v, w);
	if (c <= -2)
		return NULL;
	return convert_3way_to_object(op, c);
}

static PyObject *
do_richcmp(PyObject *v, PyObject *w, int op)
{
	PyObject *res;

	res = try_rich_compare(v, w, op);
	if (res != Py_NotImplemented)
		return res;
	Py_DECREF(res);

	return try_3way_to_rich_compare(v, w, op);
}

PyObject *
PyObject_RichCompare(PyObject *v, PyObject *w, int op)
{
        PyObject *res;

        compare_nesting++;
        if (compare_nesting > NESTING_LIMIT &&
            (v->ob_type->tp_as_mapping || v->ob_type->tp_as_sequence) &&
            !PyString_CheckExact(v) &&
            !PyTuple_CheckExact(v)) {
                /* try to detect circular data structures */
                PyObject *token = check_recursion(v, w, op);
                if (token == NULL) {
                        res = NULL;
                        goto Done;
                }
                else if (token == Py_None) {
                        /* already comparing these objects with this operator.
                           assume they're equal until shown otherwise */
                        if (op == Py_EQ)
                                res = Py_True;
                        else if (op == Py_NE)
                                res = Py_False;
                        else {
				/* ERROR */
                                res = NULL;
                        }
                        Py_XINCREF(res);
                }
                else {
                        res = do_richcmp(v, w, op);
                        delete_token(token);
                }
                goto Done;
        }

        /* No nesting extremism.
           If the types are equal, and not old-style instances, try to
           get out cheap (don't bother with coercions etc.). */
        if (v->ob_type == w->ob_type && !PyInstance_Check(v)) {
                cmpfunc fcmp;
                richcmpfunc frich = RICHCOMPARE(v->ob_type);
                /* If the type has richcmp, try it first.  try_rich_compare
                   tries it two-sided, which is not needed since we've a
                   single type only. */
                if (frich != NULL) {
                        res = (*frich)(v, w, op);
                        if (res != Py_NotImplemented)
                                goto Done;
                        Py_DECREF(res);
                }
                /* No richcmp, or this particular richmp not implemented.
                   Try 3-way cmp. */
                fcmp = v->ob_type->tp_compare;
                if (fcmp != NULL) {
                        int c = (*fcmp)(v, w);
                        if (c == -2) {
                                res = NULL;
                                goto Done;
                        }
                        res = convert_3way_to_object(op, c);
                        goto Done;
                }
        }

        /* Fast path not taken, or couldn't deliver a useful result. */
        res = do_richcmp(v, w, op);
Done:
	compare_nesting--;
	return res;
}

int
PyObject_RichCompareBool(PyObject *v, PyObject *w, int op)
{
	PyObject *res = PyObject_RichCompare(v, w, op);
	int ok;

	if (res == NULL)
		return -1;
	if (PyBool_Check(res))
		ok = (res == Py_True);
	else
		ok = PyObject_IsTrue(res);
	Py_DECREF(res);
	return ok;
}

static int
internal_print(PyObject *op)
{
  return (*op->ob_type->tp_print)(op);
}

int
PyObject_Print(PyObject *op)
{
  return internal_print(op);
}

long
PyObject_Hash(PyObject *v)
{
	PyTypeObject *tp = v->ob_type;
	if (tp->tp_hash != NULL)
		return (*tp->tp_hash)(v);
	/* TO DO */
	return -1;
}

/* ... */

static PyTypeObject PyNone_Type = {
  PyObject_HEAD_INIT(&PyType_Type)
  0,
  "NoneType",
  0,
  0,
  0, //(destructor)none_dealloc,	     /*tp_dealloc*/ /*never called*/
  0,		/*tp_print*/
  0,		/*tp_getattr*/
  0,		/*tp_setattr*/
  0,		/*tp_compare*/
  0, //(reprfunc)none_repr, /*tp_repr*/
  0,		/*tp_as_number*/
  0,		/*tp_as_sequence*/
  0,		/*tp_as_mapping*/
  0,		/*tp_hash */
};

PyObject _Py_NoneStruct = {
  PyObject_HEAD_INIT(&PyNone_Type)
};

static PyTypeObject PyNotImplemented_Type = {
  PyObject_HEAD_INIT(&PyType_Type)
  0,
  "NotImplementedType",
  0,
  0,
  0, //(destructor)none_dealloc,	     /*tp_dealloc*/ /*never called*/
  0,		/*tp_print*/
  0,		/*tp_getattr*/
  0,		/*tp_setattr*/
  0,		/*tp_compare*/
  0, //(reprfunc)NotImplemented_repr, /*tp_repr*/
  0,		/*tp_as_number*/
  0,		/*tp_as_sequence*/
  0,		/*tp_as_mapping*/
  0,		/*tp_hash */
};

PyObject _Py_NotImplementedStruct = {
	PyObject_HEAD_INIT(&PyNotImplemented_Type)
};

int
PyObject_IsTrue(PyObject *v)
{
	int res;
	if (v == Py_True)
		return 1;
	if (v == Py_False)
		return 0;
	if (v == Py_None)
		return 0;
	else if (v->ob_type->tp_as_number != NULL &&
		 v->ob_type->tp_as_number->nb_nonzero != NULL)
		res = (*v->ob_type->tp_as_number->nb_nonzero)(v);
	else if (v->ob_type->tp_as_mapping != NULL &&
		 v->ob_type->tp_as_mapping->mp_length != NULL)
		res = (*v->ob_type->tp_as_mapping->mp_length)(v);
	else if (v->ob_type->tp_as_sequence != NULL &&
		 v->ob_type->tp_as_sequence->sq_length != NULL)
		res = (*v->ob_type->tp_as_sequence->sq_length)(v);
	else
		return 1;
	return (res > 0) ? 1 : res;
}

int
PyNumber_CoerceEx(PyObject **pv, PyObject **pw)
{
	register PyObject *v = *pv;
	register PyObject *w = *pw;
	int res;

	/* Shortcut only for old-style types */
	if (v->ob_type == w->ob_type &&
	    !PyType_HasFeature(v->ob_type, Py_TPFLAGS_CHECKTYPES))
	{
		Py_INCREF(v);
		Py_INCREF(w);
		return 0;
	}
	if (v->ob_type->tp_as_number && v->ob_type->tp_as_number->nb_coerce) {
		res = (*v->ob_type->tp_as_number->nb_coerce)(pv, pw);
		if (res <= 0)
			return res;
	}
	if (w->ob_type->tp_as_number && w->ob_type->tp_as_number->nb_coerce) {
		res = (*w->ob_type->tp_as_number->nb_coerce)(pw, pv);
		if (res <= 0)
			return res;
	}
	return 1;
}
