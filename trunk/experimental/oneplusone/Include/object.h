#ifndef Py_OBJECT_H
#define Py_OBJECT_H

#define Py_LT 0
#define Py_LE 1
#define Py_EQ 2
#define Py_NE 3
#define Py_GT 4
#define Py_GE 5

#define PyObject_HEAD_INIT(type)	\
	1, type,

#define PyObject_HEAD			\
	int ob_refcnt;			\
	struct _typeobject *ob_type;

#define PyObject_VAR_HEAD		\
	PyObject_HEAD			\
	int ob_size; /* Number of items in variable part */


typedef struct _object {
	PyObject_HEAD
} PyObject;

typedef struct {
  PyObject_VAR_HEAD
} PyVarObject;

typedef PyObject * (*unaryfunc)(PyObject *);
typedef PyObject * (*binaryfunc)(PyObject *, PyObject *);

typedef int (*inquiry)(PyObject *);
typedef int (*coercion)(PyObject **, PyObject **);

typedef int (*getreadbufferproc)(PyObject *, int, void **);
typedef int (*getwritebufferproc)(PyObject *, int, void **);
typedef int (*getsegcountproc)(PyObject *, int *);
typedef int (*getcharbufferproc)(PyObject *, int, const char **);

typedef struct {
  /* TO DO */
  binaryfunc nb_add;
  inquiry nb_nonzero;

  coercion nb_coerce;

  unaryfunc nb_int;

  unaryfunc nb_float;

} PyNumberMethods;

typedef struct {
  inquiry sq_length;
  binaryfunc sq_concat;
} PySequenceMethods;

typedef struct {
  inquiry mp_length;
} PyMappingMethods;

typedef struct {
	getreadbufferproc bf_getreadbuffer;
	getwritebufferproc bf_getwritebuffer;
	getsegcountproc bf_getsegcount;
	getcharbufferproc bf_getcharbuffer;
} PyBufferProcs;



PyAPI_DATA(PyObject) _Py_NoneStruct;
#define Py_None (&_Py_NoneStruct)

PyAPI_DATA(PyObject) _Py_NotImplementedStruct;
#define Py_NotImplemented (&_Py_NotImplementedStruct)


typedef void (*destructor)(PyObject *);
typedef int (*printfunc)(PyObject *);
typedef long (*hashfunc)(PyObject *);
typedef int(*cmpfunc)(PyObject *, PyObject *);
typedef PyObject *(*richcmpfunc) (PyObject *, PyObject *, int);

typedef struct _typeobject {
  PyObject_VAR_HEAD
  char *tp_name;
  int tp_basicsize, tp_itemsize;

  destructor tp_dealloc;
  printfunc tp_print;
  cmpfunc tp_compare;

  PyNumberMethods *tp_as_number;
  PySequenceMethods *tp_as_sequence;
  PyMappingMethods *tp_as_mapping;

  hashfunc tp_hash;

  PyBufferProcs *tp_as_buffer;

  long tp_flags;

  richcmpfunc tp_richcompare;

  struct _typeobject *tp_base;

} PyTypeObject;

PyAPI_FUNC(int) PyNumber_CoerceEx(PyObject **, PyObject **);

extern int _PyObject_SlotCompare(PyObject *, PyObject *);

#define _Py_NewReference(op) (			\
	(op)->ob_refcnt = 1)

#define _Py_Dealloc(op) 

/* @@@ ((*(op)->ob_type->tp_dealloc)((PyObject *)(op))) */

#define Py_INCREF(op) (				\
	(op)->ob_refcnt++)

#define Py_DECREF(op)				\
	if (--(op)->ob_refcnt != 0)		\
		;				\
	else					\
		_Py_Dealloc((PyObject *)(op))

#define Py_XINCREF(op) if ((op) == NULL) ; else Py_INCREF(op)
#define Py_XDECREF(op) if ((op) == NULL) ; else Py_DECREF(op)

PyAPI_DATA(PyTypeObject) PyType_Type;
PyAPI_DATA(PyTypeObject) PyBaseObject_Type;

PyAPI_FUNC(int) PyType_IsSubtype(PyTypeObject *, PyTypeObject *);
#define PyObject_TypeCheck(ob, tp) \
	((ob)->ob_type == (tp) || PyType_IsSubtype((ob)->ob_type, (tp)))

PyAPI_FUNC(int) PyObject_Print(PyObject *);

PyAPI_FUNC(long) PyObject_Hash(PyObject *);
PyAPI_FUNC(int) PyObject_IsTrue(PyObject *);

PyAPI_FUNC(PyObject *) PyObject_RichCompare(PyObject *, PyObject *, int);
PyAPI_FUNC(int) PyObject_RichCompareBool(PyObject *, PyObject *, int);

#define Py_TPFLAGS_CHECKTYPES (1L<<4)
#define Py_TPFLAGS_HAVE_RICHCOMPARE (1L<<5)
#define Py_TPFLAGS_HAVE_CLASS (1L<<8)

#define PyType_HasFeature(t,f) (((t)->tp_flags & (f)) != 0)

#endif /* !Py_OBJECT_H */
