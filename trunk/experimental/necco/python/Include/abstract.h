#ifndef Py_ABSTRACTOBJECT_H
#define Py_ABSTRACTOBJECT_H

     PyAPI_FUNC(int) PyObject_Size(PyObject *o);

       /*
         Return the size of object o.  If the object, o, provides
	 both sequence and mapping protocols, the sequence size is
	 returned. On error, -1 is returned.  This is the equivalent
	 to the Python expression: len(o).

       */

     PyAPI_FUNC(PyObject *) PyNumber_Remainder(PyObject *o1, PyObject *o2);

       /*
	 Returns the remainder of dividing o1 by o2, or null on
	 failure.  This is the equivalent of the Python expression:
	 o1%o2.
       */

     PyAPI_FUNC(PyObject *) PyNumber_And(PyObject *o1, PyObject *o2);

       /*
	 Returns the result of bitwise and of o1 and o2 on success, or
	 NULL on failure. This is the equivalent of the Python
	 expression: o1&o2.
       */

     PyAPI_FUNC(PyObject *) PyObject_GetItem(PyObject *o, PyObject *key);

       /*
	 Return element of o corresponding to the object, key, or NULL
	 on failure. This is the equivalent of the Python expression:
	 o[key].
       */

     PyAPI_FUNC(PyObject *) PyObject_Call(PyObject *callable_object,
					 PyObject *args, PyObject *kw);

       /*
	 Call a callable Python object, callable_object, with
	 arguments and keywords arguments.  The 'args' argument can not be
	 NULL, but the 'kw' argument can be NULL.
       */

     PyAPI_FUNC(PyObject *) PyObject_GetIter(PyObject *);
     /* Takes an object and returns an iterator for it.
        This is typically a new iterator but if the argument
	is an iterator, this returns itself. */

#define PyIter_Check(obj) \
    (PyType_HasFeature((obj)->ob_type, Py_TPFLAGS_HAVE_ITER) && \
     (obj)->ob_type->tp_iternext != NULL)

     PyAPI_FUNC(PyObject *) PyIter_Next(PyObject *);
     /* Takes an iterator object and calls its tp_iternext slot,
	returning the next value.  If the iterator is exhausted,
	this returns NULL without setting an exception.
	NULL with an exception means an error occurred. */

     PyAPI_FUNC(int) PySequence_Size(PyObject *o);

       /*
         Return the size of sequence object o, or -1 on failure.

       */

     PyAPI_FUNC(PyObject *) PySequence_GetItem(PyObject *o, int i);

       /*
	 Return the ith element of o, or NULL on failure. This is the
	 equivalent of the Python expression: o[i].
       */

     PyAPI_FUNC(PyObject *) PySequence_GetSlice(PyObject *o, int i1, int i2);

       /*
	 Return the slice of sequence object o between i1 and i2, or
	 NULL on failure. This is the equivalent of the Python
	 expression: o[i1:i2].

       */

     PyAPI_FUNC(PyObject *) PySequence_Fast(PyObject *o, const char* m);
       /*
         Returns the sequence, o, as a tuple, unless it's already a
         tuple or list.  Use PySequence_Fast_GET_ITEM to access the
         members of this list, and PySequence_Fast_GET_SIZE to get its length.

         Returns NULL on failure.  If the object does not support iteration,
         raises a TypeError exception with m as the message text.
       */

#define PySequence_Fast_GET_SIZE(o) \
	(PyList_Check(o) ? PyList_GET_SIZE(o) : PyTuple_GET_SIZE(o))
       /*
	 Return the size of o, assuming that o was returned by
         PySequence_Fast and is not NULL.
       */

#define PySequence_Fast_GET_ITEM(o, i)\
     (PyList_Check(o) ? PyList_GET_ITEM(o, i) : PyTuple_GET_ITEM(o, i))
       /*
	 Return the ith element of o, assuming that o was returned by
         PySequence_Fast, and that i is within bounds.
       */


#endif /* Py_ABSTRACTOBJECT_H */
