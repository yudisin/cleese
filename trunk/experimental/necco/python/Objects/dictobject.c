
#include "Python.h"

typedef PyDictEntry dictentry;
typedef PyDictObject dictobject;

#define PERTURB_SHIFT 5

static PyObject *dummy;

/* forward declarations */
static dictentry *
lookdict_string(dictobject *mp, PyObject *key, long hash);

#define INIT_NONZERO_DICT_SLOTS(mp) do {			\
		(mp)->ma_table = (mp)->ma_smalltable;		\
		(mp)->ma_mask = PyDict_MINSIZE -1;		\
	} while(0)

#define EMPTY_TO_MINSIZE(mp) do {					\
		memset((mp)->ma_smalltable, 0, sizeof((mp)->ma_smalltable)); \
		(mp)->ma_used = (mp)->ma_fill = 0;			\
		INIT_NONZERO_DICT_SLOTS(mp);				\
	} while(0)

PyObject *
PyDict_New(void)
{
	LOG("> PyDict_New\n"); {

	register dictobject *mp;
	if (dummy == NULL) {
		dummy = PyString_FromString("<dummy key>");
		if (dummy == NULL)
			return NULL;
	}
	mp = PyObject_GC_New(dictobject, &PyDict_Type);
	if (mp == NULL)
		return NULL;
	EMPTY_TO_MINSIZE(mp);
	mp->ma_lookup = lookdict_string;
	_PyObject_GC_TRACK(mp);

	LOG("< PyDict_New\n");
	return (PyObject *)mp;
}}

static dictentry *
lookdict_string(dictobject *mp, PyObject *key, register long hash)
{
	register int i;
	register unsigned int perturb;
	register dictentry *freeslot;
	register unsigned int mask = mp->ma_mask;
	dictentry *ep0 = mp->ma_table;
	register dictentry *ep;

	i = hash & mask;
	ep = &ep0[i];
	if (ep->me_key == NULL || ep->me_key == key)
		return ep;
	if (ep->me_key == dummy)
		freeslot = ep;
	else {
		if (ep->me_hash == hash
		    && _PyString_Eq(ep->me_key, key)) {
			return ep;
		}
		freeslot = NULL;
	}

        for (perturb = hash; ; perturb >>= PERTURB_SHIFT) {
                i = (i << 2) + i + perturb + 1;
                ep = &ep0[i & mask];
                if (ep->me_key == NULL)
                        return freeslot == NULL ? ep : freeslot;
                if (ep->me_key == key
                    || (ep->me_hash == hash
                        && ep->me_key != dummy
                        && _PyString_Eq(ep->me_key, key)))
                        return ep;
                if (ep->me_key == dummy && freeslot == NULL)
                        freeslot = ep;
        }
}

static void
insertdict(register dictobject *mp, PyObject *key, long hash, PyObject *value)
{
	PyObject *old_value;
	register dictentry *ep;
	typedef PyDictEntry *(*lookupfunc)(PyDictObject *, PyObject *, long);

	ep = mp->ma_lookup(mp, key, hash);
	if (ep->me_value != NULL) {
		old_value = ep->me_value;
		ep->me_value = value;
		Py_DECREF(old_value);
		Py_DECREF(key);
	}
	else {
		if (ep->me_key == NULL)
			mp->ma_fill++;
		else
			Py_DECREF(ep->me_key);
		ep->me_key = key;
		ep->me_hash = hash;
		ep->me_value = value;
		mp->ma_used++;
	}
}

static int
dictresize(dictobject *mp, int minused)
{
	int newsize;
	dictentry *oldtable, *newtable, *ep;
	int i;
	int is_oldtable_malloced;
	dictentry small_copy[PyDict_MINSIZE];

	for (newsize = PyDict_MINSIZE;
             newsize <= minused && newsize > 0;
             newsize <<= 1)
		;
	if (newsize <= 0) {
		/* NO MEMORY */
		return -1;
	}

	oldtable = mp->ma_table;
	is_oldtable_malloced = oldtable != mp->ma_smalltable;

	if (newsize == PyDict_MINSIZE) {
		newtable = mp->ma_smalltable;
		if (newtable == oldtable) {
			if (mp->ma_fill == mp->ma_used) {
				return 0;
			}
			memcpy(small_copy, oldtable, sizeof(small_copy));
			oldtable = small_copy;
		}
	}
	else {
		newtable = PyMem_NEW(dictentry, newsize);
		if (newtable == NULL) {
			/* NO MEMORY */
			return -1;
		}
	}

	mp->ma_table = newtable;
	mp->ma_mask = newsize -1;
	memset(newtable, 0, sizeof(dictentry) * newsize);
	mp->ma_used = 0;
	i = mp->ma_fill;
	mp->ma_fill = 0;

	for (ep = oldtable; i > 0; ep++) {
		if (ep->me_value != NULL) {
			--i;
			insertdict(mp, ep->me_key, ep->me_hash, ep->me_value);
		}
		else if (ep->me_key != NULL) {
			--i;
			Py_DECREF(ep->me_key);
		}
	}

	return 0;
}

PyObject *
PyDict_GetItem(PyObject *op, PyObject *key)
{
	long hash;
	dictobject *mp = (dictobject *)op;
	if (!PyDict_Check(op)) {
		return NULL;
	}
	if (!PyString_CheckExact(key) ||
	    (hash = ((PyStringObject *) key)->ob_shash) == -1)
	{
		hash = PyObject_Hash(key);
		if (hash == -1) {
			return NULL;
		}
	}
	{ PyObject *ret = (mp->ma_lookup)(mp, key, hash)->me_value;
	return ret; }
}

int
PyDict_SetItem(register PyObject *op, PyObject *key, PyObject *value)
{
	register dictobject *mp;
	register long hash;
	register int n_used;

	if (!PyDict_Check(op)) {
		/* ERROR */
		return -1;
	}
	mp = (dictobject *)op;
	if (PyString_CheckExact(key)) {
		hash = ((PyStringObject *)key)->ob_shash;
		if (hash == -1)
			hash = PyObject_Hash(key);
	}
	else {
		hash = PyObject_Hash(key);
		if (hash == -1) 
			return -1;
	}
	n_used = mp->ma_used;
	Py_INCREF(value);
	Py_INCREF(key);
	insertdict(mp, key, hash, value);
	if (!(mp->ma_used > n_used && mp->ma_fill*3 >= (mp->ma_mask+1)*2))
		return 0;
	return dictresize(mp, mp->ma_used*(mp->ma_used>50000 ? 2 : 4));
}

int
PyDict_DelItem(PyObject *op, PyObject *key)
{
	register dictobject *mp;
	register long hash;
	register dictentry *ep;
	PyObject *old_value, *old_key;

	if (!PyDict_Check(op)) {
	  /* ERROR */
		return -1;
	}
	if (!PyString_CheckExact(key) ||
	    (hash = ((PyStringObject *) key)->ob_shash) == -1) {
		hash = PyObject_Hash(key);
		if (hash == -1)
			return -1;
	}
	mp = (dictobject *)op;
	ep = (mp->ma_lookup)(mp, key, hash);
	if (ep->me_value == NULL) {
	  /* ERROR */
		return -1;
	}
	old_key = ep->me_key;
	Py_INCREF(dummy);
	ep->me_key = dummy;
	old_value = ep->me_value;
	ep->me_value = NULL;
	mp->ma_used--;
	Py_DECREF(old_value);
	Py_DECREF(old_key);
	return 0;
}

void
PyDict_Clear(PyObject *op)
{
	dictobject *mp;
	dictentry *ep, *table;
	int table_is_malloced;
	int fill;
	dictentry small_copy[PyDict_MINSIZE];

	if (!PyDict_Check(op))
		return;
	mp = (dictobject *)op;

	table = mp->ma_table;
	assert(table != NULL);
	table_is_malloced = table != mp->ma_smalltable;

	/* This is delicate.  During the process of clearing the dict,
	 * decrefs can cause the dict to mutate.  To avoid fatal confusion
	 * (voice of experience), we have to make the dict empty before
	 * clearing the slots, and never refer to anything via mp->xxx while
	 * clearing.
	 */
	fill = mp->ma_fill;
	if (table_is_malloced)
		EMPTY_TO_MINSIZE(mp);

	else if (fill > 0) {
		/* It's a small table with something that needs to be cleared.
		 * Afraid the only safe way is to copy the dict entries into
		 * another small table first.
		 */
		memcpy(small_copy, table, sizeof(small_copy));
		table = small_copy;
		EMPTY_TO_MINSIZE(mp);
	}
	/* else it's a small table that's already empty */

	/* Now we can finally clear things.  If C had refcounts, we could
	 * assert that the refcount on table is 1 now, i.e. that this function
	 * has unique access to it, so decref side-effects can't alter it.
	 */
	for (ep = table; fill > 0; ++ep) {
		if (ep->me_key) {
			--fill;
			Py_DECREF(ep->me_key);
			Py_XDECREF(ep->me_value);
		}
	}

	if (table_is_malloced)
		PyMem_DEL(table);
}

/*
 * Iterate over a dict.  Use like so:
 *
 *     int i;
 *     PyObject *key, *value;
 *     i = 0;   # important!  i should not otherwise be changed by you
 *     while (PyDict_Next(yourdict, &i, &key, &value)) {
 *              Refer to borrowed references in key and value.
 *     }
 *
 * CAUTION:  In general, it isn't safe to use PyDict_Next in a loop that
 * mutates the dict.  One exception:  it is safe if the loop merely changes
 * the values associated with the keys (but doesn't insert new keys or
 * delete keys), via PyDict_SetItem().
 */
int
PyDict_Next(PyObject *op, int *ppos, PyObject **pkey, PyObject **pvalue)
{
	int i;
	register dictobject *mp;
	if (!PyDict_Check(op))
		return 0;
	mp = (dictobject *)op;
	i = *ppos;
	if (i < 0)
		return 0;
	while (i <= mp->ma_mask && mp->ma_table[i].me_value == NULL)
		i++;
	*ppos = i+1;
	if (i > mp->ma_mask)
		return 0;
	if (pkey)
		*pkey = mp->ma_table[i].me_key;
	if (pvalue)
		*pvalue = mp->ma_table[i].me_value;
	return 1;
}

PyTypeObject PyDict_Type = {
	PyObject_HEAD_INIT(&PyType_Type)
	0,
	"dict",
	sizeof(dictobject),
	0,
	0, //(destructor)dict_dealloc,		/* tp_dealloc */
	0, //(printfunc)dict_print,			/* tp_print */
	0,					/* tp_getattr */
	0,					/* tp_setattr */
	0, //(cmpfunc)dict_compare,			/* tp_compare */
	0, //(reprfunc)dict_repr,			/* tp_repr */
	0,					/* tp_as_number */
	0, //&dict_as_sequence,			/* tp_as_sequence */
	0, //&dict_as_mapping,			/* tp_as_mapping */
	0, //dict_nohash,				/* tp_hash */
	0,					/* tp_call */
	0,					/* tp_str */
	PyObject_GenericGetAttr,		/* tp_getattro */
	0,					/* tp_setattro */
	0,					/* tp_as_buffer */
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_GC | Py_TPFLAGS_BASETYPE,		/* tp_flags */
	0, //dictionary_doc,				/* tp_doc */
	0, //(traverseproc)dict_traverse,		/* tp_traverse */
	0, //(inquiry)dict_tp_clear,			/* tp_clear */
	0, //dict_richcompare,			/* tp_richcompare */
	0,					/* tp_weaklistoffset */
	0, //(getiterfunc)dict_iter,			/* tp_iter */
	0,					/* tp_iternext */
	0, //mapp_methods,				/* tp_methods */
	0,					/* tp_members */
	0,					/* tp_getset */
	0,					/* tp_base */
	0,					/* tp_dict */
	0,					/* tp_descr_get */
	0,					/* tp_descr_set */
	0,					/* tp_dictoffset */
	0, //(initproc)dict_init,			/* tp_init */
	0, //PyType_GenericAlloc,			/* tp_alloc */
	0, //dict_new,				/* tp_new */
	0, //PyObject_GC_Del,        		/* tp_free */
};

PyObject *
PyDict_GetItemString(PyObject *v, const char *key)
{
	LOG("> PyDict_GetItemString\n"); {
	PyObject *kv, *rv;
	kv = PyString_FromString(key);
	if (kv == NULL)
		return NULL;
	rv = PyDict_GetItem(v, kv);
	Py_DECREF(kv);
	LOG("< PyDict_GetItemString\n");
	return rv;
}}

int
PyDict_SetItemString(PyObject *v, const char *key, PyObject *item)
{
	PyObject *kv;
	int err;
	kv = PyString_FromString(key);
	if (kv == NULL)
		return -1;
	PyString_InternInPlace(&kv);
	err = PyDict_SetItem(v, kv, item);
	Py_DECREF(kv);
	return err;
}

PyObject *
PyDict_Copy(PyObject *o)
{
	register dictobject *mp;
	register int i;
	dictobject *copy;
	dictentry *entry;

	if (o == NULL || !PyDict_Check(o)) {
		/* ERROR */
		return NULL;
	}
	mp = (dictobject *)o;
	copy = (dictobject *)PyDict_New();
	if (copy == NULL)
		return NULL;
	if (mp->ma_used > 0) {
		if (dictresize(copy, mp->ma_used*2) != 0)
			return NULL;
		for (i = 0; i <= mp->ma_mask; i++) {
			entry = &mp->ma_table[i];
			if (entry->me_value != NULL) {
				Py_INCREF(entry->me_key);
				Py_INCREF(entry->me_value);
				insertdict(copy, entry->me_key, entry->me_hash,
					   entry->me_value);
			}
		}
	}
	return (PyObject *)copy;
}

int
PyDict_Size(PyObject *mp)
{
	if (mp == NULL || !PyDict_Check(mp)) {
		/* ERROR */
		return 0;
	}
	return ((dictobject *)mp)->ma_used;
}
