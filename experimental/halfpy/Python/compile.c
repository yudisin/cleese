/* Compile an expression node to intermediate code */

/* XXX TO DO:
   XXX add __doc__ attribute == co_doc to code object attributes?
   XXX   (it's currently the first item of the co_const tuple)
   XXX Generate simple jump for break/return outside 'try...finally'
   XXX Allow 'continue' inside finally clause of try-finally
   XXX New opcode for loading the initial index for a for loop
   XXX other JAR tricks?
*/

#include "Python.h"

#include "compile.h"
#include "opcode.h"
#include "structmember.h"

#include <ctype.h>


int Py_OptimizeFlag = 0;


/* ... */



#define OFF(x) offsetof(PyCodeObject, x)

static PyMemberDef code_memberlist[] = {
	{"co_argcount",	T_INT,		OFF(co_argcount),	READONLY},
	{"co_nlocals",	T_INT,		OFF(co_nlocals),	READONLY},
	{"co_stacksize",T_INT,		OFF(co_stacksize),	READONLY},
	{"co_flags",	T_INT,		OFF(co_flags),		READONLY},
	{"co_code",	T_OBJECT,	OFF(co_code),		READONLY},
	{"co_consts",	T_OBJECT,	OFF(co_consts),		READONLY},
	{"co_names",	T_OBJECT,	OFF(co_names),		READONLY},
	{"co_varnames",	T_OBJECT,	OFF(co_varnames),	READONLY},
	{"co_freevars",	T_OBJECT,	OFF(co_freevars),	READONLY},
	{"co_cellvars",	T_OBJECT,	OFF(co_cellvars),	READONLY},
	{"co_filename",	T_OBJECT,	OFF(co_filename),	READONLY},
	{"co_name",	T_OBJECT,	OFF(co_name),		READONLY},
	{"co_firstlineno", T_INT,	OFF(co_firstlineno),	READONLY},
	{"co_lnotab",	T_OBJECT,	OFF(co_lnotab),		READONLY},
	{NULL}	/* Sentinel */
};


PyDoc_STRVAR(code_doc,
"code(argcount, nlocals, stacksize, flags, codestring, constants, names,\n\
      varnames, filename, name, firstlineno, lnotab[, freevars[, cellvars]])\n\
\n\
Create a code object.  Not for the faint of heart.");

static PyObject *
code_new(PyTypeObject *type, PyObject *args, PyObject *kw)
{
	int argcount;
	int nlocals;
	int stacksize;
	int flags;
	PyObject *code;
	PyObject *consts;
	PyObject *names;
	PyObject *varnames;
	PyObject *freevars = NULL;
	PyObject *cellvars = NULL;
	PyObject *filename;
	PyObject *name;
	int firstlineno;
	PyObject *lnotab;

	if (!PyArg_ParseTuple(args, "iiiiSO!O!O!SSiS|O!O!:code",
			      &argcount, &nlocals, &stacksize, &flags,
			      &code,
			      &PyTuple_Type, &consts,
			      &PyTuple_Type, &names,
			      &PyTuple_Type, &varnames,
			      &filename, &name,
			      &firstlineno, &lnotab,
			      &PyTuple_Type, &freevars,
			      &PyTuple_Type, &cellvars))
		return NULL;

	if (freevars == NULL || cellvars == NULL) {
		PyObject *empty = PyTuple_New(0);
		if (empty == NULL)
		    return NULL;
		if (freevars == NULL) {
		    freevars = empty;
		    Py_INCREF(freevars);
		}
		if (cellvars == NULL) {
		    cellvars = empty;
		    Py_INCREF(cellvars);
		}
		Py_DECREF(empty);
	}

	if (!PyObject_CheckReadBuffer(code)) {
		PyErr_SetString(PyExc_TypeError,
		  "bytecode object must be a single-segment read-only buffer");
		return NULL;
	}

	return (PyObject *)PyCode_New(argcount, nlocals, stacksize, flags,
				      code, consts, names, varnames,
				      freevars, cellvars, filename, name,
				      firstlineno, lnotab); 
}

static void
code_dealloc(PyCodeObject *co)
{
	Py_XDECREF(co->co_code);
	Py_XDECREF(co->co_consts);
	Py_XDECREF(co->co_names);
	Py_XDECREF(co->co_varnames);
	Py_XDECREF(co->co_freevars);
	Py_XDECREF(co->co_cellvars);
	Py_XDECREF(co->co_filename);
	Py_XDECREF(co->co_name);
	Py_XDECREF(co->co_lnotab);
	PyObject_DEL(co);
}

static PyObject *
code_repr(PyCodeObject *co)
{
	char buf[500];
	int lineno = -1;
	char *filename = "???";
	char *name = "???";

	if (co->co_firstlineno != 0)
		lineno = co->co_firstlineno;
	if (co->co_filename && PyString_Check(co->co_filename))
		filename = PyString_AS_STRING(co->co_filename);
	if (co->co_name && PyString_Check(co->co_name))
		name = PyString_AS_STRING(co->co_name);
	PyOS_snprintf(buf, sizeof(buf),
		      "<code object %.100s at %p, file \"%.300s\", line %d>",
		      name, co, filename, lineno);
	return PyString_FromString(buf);
}

static int
code_compare(PyCodeObject *co, PyCodeObject *cp)
{
	int cmp;
	cmp = PyObject_Compare(co->co_name, cp->co_name);
	if (cmp) return cmp;
	cmp = co->co_argcount - cp->co_argcount;
	if (cmp) return (cmp<0)?-1:1;
	cmp = co->co_nlocals - cp->co_nlocals;
	if (cmp) return (cmp<0)?-1:1;
	cmp = co->co_flags - cp->co_flags;
	if (cmp) return (cmp<0)?-1:1;
	cmp = PyObject_Compare(co->co_code, cp->co_code);
	if (cmp) return cmp;
	cmp = PyObject_Compare(co->co_consts, cp->co_consts);
	if (cmp) return cmp;
	cmp = PyObject_Compare(co->co_names, cp->co_names);
	if (cmp) return cmp;
	cmp = PyObject_Compare(co->co_varnames, cp->co_varnames);
	if (cmp) return cmp;
	cmp = PyObject_Compare(co->co_freevars, cp->co_freevars);
	if (cmp) return cmp;
	cmp = PyObject_Compare(co->co_cellvars, cp->co_cellvars);
	return cmp;
}

static long
code_hash(PyCodeObject *co)
{
	long h, h0, h1, h2, h3, h4, h5, h6;
	h0 = PyObject_Hash(co->co_name);
	if (h0 == -1) return -1;
	h1 = PyObject_Hash(co->co_code);
	if (h1 == -1) return -1;
	h2 = PyObject_Hash(co->co_consts);
	if (h2 == -1) return -1;
	h3 = PyObject_Hash(co->co_names);
	if (h3 == -1) return -1;
	h4 = PyObject_Hash(co->co_varnames);
	if (h4 == -1) return -1;
	h5 = PyObject_Hash(co->co_freevars);
	if (h5 == -1) return -1;
	h6 = PyObject_Hash(co->co_cellvars);
	if (h6 == -1) return -1;
	h = h0 ^ h1 ^ h2 ^ h3 ^ h4 ^ h5 ^ h6 ^
		co->co_argcount ^ co->co_nlocals ^ co->co_flags;
	if (h == -1) h = -2;
	return h;
}

/* XXX code objecats need to participate in GC? */

PyTypeObject PyCode_Type = {
	PyObject_HEAD_INIT(&PyType_Type)
	0,
	"code",
	sizeof(PyCodeObject),
	0,
	(destructor)code_dealloc, 	/* tp_dealloc */
	0,				/* tp_print */
	0, 				/* tp_getattr */
	0,				/* tp_setattr */
	(cmpfunc)code_compare, 		/* tp_compare */
	(reprfunc)code_repr,		/* tp_repr */
	0,				/* tp_as_number */
	0,				/* tp_as_sequence */
	0,				/* tp_as_mapping */
	(hashfunc)code_hash, 		/* tp_hash */
	0,				/* tp_call */
	0,				/* tp_str */
	PyObject_GenericGetAttr,	/* tp_getattro */
	0,				/* tp_setattro */
	0,				/* tp_as_buffer */
	Py_TPFLAGS_DEFAULT,		/* tp_flags */
	code_doc,			/* tp_doc */
	0,				/* tp_traverse */
	0,				/* tp_clear */
	0,				/* tp_richcompare */
	0,				/* tp_weaklistoffset */
	0,				/* tp_iter */
	0,				/* tp_iternext */
	0,				/* tp_methods */
	code_memberlist,		/* tp_members */
	0,				/* tp_getset */
	0,				/* tp_base */
	0,				/* tp_dict */
	0,				/* tp_descr_get */
	0,				/* tp_descr_set */
	0,				/* tp_dictoffset */
	0,				/* tp_init */
	0,				/* tp_alloc */
	code_new,			/* tp_new */
};

#define NAME_CHARS \
	"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz"

/* all_name_chars(s): true iff all chars in s are valid NAME_CHARS */

static int
all_name_chars(unsigned char *s)
{
	static char ok_name_char[256];
	static unsigned char *name_chars = (unsigned char *)NAME_CHARS;

	if (ok_name_char[*name_chars] == 0) {
		unsigned char *p;
		for (p = name_chars; *p; p++)
			ok_name_char[*p] = 1;
	}
	while (*s) {
		if (ok_name_char[*s++] == 0)
			return 0;
	}
	return 1;
}

static int
intern_strings(PyObject *tuple)
{
	int i;

	for (i = PyTuple_GET_SIZE(tuple); --i >= 0; ) {
		PyObject *v = PyTuple_GET_ITEM(tuple, i);
		if (v == NULL || !PyString_Check(v)) {
			Py_FatalError("non-string found in code slot");
			PyErr_BadInternalCall();
			return -1;
		}
		PyString_InternInPlace(&PyTuple_GET_ITEM(tuple, i));
	}
	return 0;
}

#define GETARG(arr, i) ((int)((arr[i+2]<<8) + arr[i+1]))
#define UNCONDITIONAL_JUMP(op)  (op==JUMP_ABSOLUTE || op==JUMP_FORWARD)
#define ABSOLUTE_JUMP(op) (op==JUMP_ABSOLUTE || op==CONTINUE_LOOP)
#define GETJUMPTGT(arr, i) (GETARG(arr,i) + (ABSOLUTE_JUMP(arr[i]) ? 0 : i+3))
#define SETARG(arr, i, val) arr[i+2] = val>>8; arr[i+1] = val & 255

static PyObject *
optimize_code(PyObject *code, PyObject* consts)
{
	int i, j, codelen;
	int tgt, tgttgt, opcode;
	unsigned char *codestr;

	/* Make a modifiable copy of the code string */
	if (!PyString_Check(code))
		goto exitUnchanged;
	codelen = PyString_Size(code);
	codestr = PyMem_Malloc(codelen);
	if (codestr == NULL) 
		goto exitUnchanged;
	codestr = memcpy(codestr, PyString_AS_STRING(code), codelen);
	assert(PyTuple_Check(consts));

	for (i=0 ; i<codelen-7 ; i += HAS_ARG(codestr[i]) ? 3 : 1) {
		opcode = codestr[i];
		switch (opcode) {

		/* Skip over LOAD_CONST trueconst  JUMP_IF_FALSE xx  POP_TOP. 
		   Note, only the first opcode is changed, the others still
		   perform normally if they happen to be jump targets. */
		case LOAD_CONST:
			j = GETARG(codestr, i);
			if (codestr[i+3] != JUMP_IF_FALSE  ||
			    codestr[i+6] != POP_TOP  ||
			    !PyObject_IsTrue(PyTuple_GET_ITEM(consts, j)))
				continue;
			codestr[i] = JUMP_FORWARD;
			SETARG(codestr, i, 4);
			break;

		/* Replace jumps to unconditional jumps */
		case FOR_ITER:
		case JUMP_FORWARD:
		case JUMP_IF_FALSE:
		case JUMP_IF_TRUE:
		case JUMP_ABSOLUTE:
		case CONTINUE_LOOP:
		case SETUP_LOOP:
		case SETUP_EXCEPT:
		case SETUP_FINALLY:
			tgt = GETJUMPTGT(codestr, i);
			if (!UNCONDITIONAL_JUMP(codestr[tgt])) 
				continue;
			tgttgt = GETJUMPTGT(codestr, tgt);
			if (opcode == JUMP_FORWARD) /* JMP_ABS can go backwards */
				opcode = JUMP_ABSOLUTE;
			if (!ABSOLUTE_JUMP(opcode))
				tgttgt -= i + 3;     /* Calc relative jump addr */
			if (tgttgt < 0)           /* No backward relative jumps */
				 continue;
			codestr[i] = opcode;
			SETARG(codestr, i, tgttgt);
			break;

		case EXTENDED_ARG:
			PyMem_Free(codestr);
			goto exitUnchanged;
		}
	}
	code = PyString_FromStringAndSize((char *)codestr, codelen);
	PyMem_Free(codestr);
	return code;

exitUnchanged:
	Py_INCREF(code);
	return code;
}

PyCodeObject *
PyCode_New(int argcount, int nlocals, int stacksize, int flags,
	   PyObject *code, PyObject *consts, PyObject *names,
	   PyObject *varnames, PyObject *freevars, PyObject *cellvars,
	   PyObject *filename, PyObject *name, int firstlineno,
	   PyObject *lnotab) 
{
	PyCodeObject *co;
	int i;
	/* Check argument types */
	if (argcount < 0 || nlocals < 0 ||
	    code == NULL ||
	    consts == NULL || !PyTuple_Check(consts) ||
	    names == NULL || !PyTuple_Check(names) ||
	    varnames == NULL || !PyTuple_Check(varnames) ||
	    freevars == NULL || !PyTuple_Check(freevars) ||
	    cellvars == NULL || !PyTuple_Check(cellvars) ||
	    name == NULL || !PyString_Check(name) ||
	    filename == NULL || !PyString_Check(filename) ||
	    lnotab == NULL || !PyString_Check(lnotab) ||
	    !PyObject_CheckReadBuffer(code)) {
		PyErr_BadInternalCall();
		return NULL;
	}
	intern_strings(names);
	intern_strings(varnames);
	intern_strings(freevars);
	intern_strings(cellvars);
	/* Intern selected string constants */
	for (i = PyTuple_Size(consts); --i >= 0; ) {
		PyObject *v = PyTuple_GetItem(consts, i);
		if (!PyString_Check(v))
			continue;
		if (!all_name_chars((unsigned char *)PyString_AS_STRING(v)))
			continue;
		PyString_InternInPlace(&PyTuple_GET_ITEM(consts, i));
	}
	co = PyObject_NEW(PyCodeObject, &PyCode_Type);
	if (co != NULL) {
		co->co_argcount = argcount;
		co->co_nlocals = nlocals;
		co->co_stacksize = stacksize;
		co->co_flags = flags;
		co->co_code = optimize_code(code, consts);
		Py_INCREF(consts);
		co->co_consts = consts;
		Py_INCREF(names);
		co->co_names = names;
		Py_INCREF(varnames);
		co->co_varnames = varnames;
		Py_INCREF(freevars);
		co->co_freevars = freevars;
		Py_INCREF(cellvars);
		co->co_cellvars = cellvars;
		Py_INCREF(filename);
		co->co_filename = filename;
		Py_INCREF(name);
		co->co_name = name;
		co->co_firstlineno = firstlineno;
		Py_INCREF(lnotab);
		co->co_lnotab = lnotab;
		if (PyTuple_GET_SIZE(freevars) == 0 &&
		    PyTuple_GET_SIZE(cellvars) == 0)
		    co->co_flags |= CO_NOFREE;
	}
	return co;
}

/* ... */



int
_Py_Mangle(char *p, char *name, char *buffer, size_t maxlen)
{
	/* Name mangling: __private becomes _classname__private.
	   This is independent from how the name is used. */
	size_t nlen, plen;
	if (p == NULL || name == NULL || name[0] != '_' || name[1] != '_')
		return 0;
	nlen = strlen(name);
	if (nlen+2 >= maxlen)
		return 0; /* Don't mangle __extremely_long_names */
	if (name[nlen-1] == '_' && name[nlen-2] == '_')
		return 0; /* Don't mangle __whatever__ */
	/* Strip leading underscores from class name */
	while (*p == '_')
		p++;
	if (*p == '\0')
		return 0; /* Don't mangle if class is just underscores */
	plen = strlen(p);
	if (plen + nlen >= maxlen)
		plen = maxlen-nlen-2; /* Truncate class name if too long */
	/* buffer = "_" + p[:plen] + name # i.e. 1+plen+nlen bytes */
	buffer[0] = '_';
	strncpy(buffer+1, p, plen);
	strcpy(buffer+1+plen, name);
	return 1;
}

int
PyCode_Addr2Line(PyCodeObject *co, int addrq)
{
	int size = PyString_Size(co->co_lnotab) / 2;
	unsigned char *p = (unsigned char*)PyString_AsString(co->co_lnotab);
	int line = co->co_firstlineno;
	int addr = 0;
	while (--size >= 0) {
		addr += *p++;
		if (addr > addrq)
			break;
		line += *p++;
	}
	return line;
}

/* ... */



