#include "Python.h"

#include "compile.h"
#include "frameobject.h"
#include "eval.h"
#include "opcode.h"

/* The interpreter's recursion limit */

static int recursion_limit = 1000;

/* Status code for main loop (reason for stack unwind) */

enum why_code {
		WHY_NOT,	/* No error */
		WHY_EXCEPTION,	/* Exception occurred */
		WHY_RERAISE,	/* Exception re-raised by 'finally' */
		WHY_RETURN,	/* 'return' statement */
		WHY_BREAK,	/* 'break' statement */
		WHY_CONTINUE,	/* 'continue' statement */
		WHY_YIELD	/* 'yield' operator */
};

PyObject *
PyEval_EvalCode(PyCodeObject *co, PyObject *globals, PyObject *locals)
{
	/* XXX raise SystemError if globals is NULL */
	return PyEval_EvalCodeEx(co,
			  globals, locals,
			  (PyObject **)NULL, 0,
			  (PyObject **)NULL, 0,
			  (PyObject **)NULL, 0,
			  NULL);
}

static PyObject *
eval_frame(PyFrameObject *f)
{
  printf("> eval_frame\n");
	PyObject **stack_pointer; /* Next free slot in value stack */
	register unsigned char *next_instr;
	register int opcode=0;	/* Current opcode */
	register int oparg=0;	/* Current opcode argument, if any */
	register enum why_code why; /* Reason for block stack unwind */
	register int err;	/* Error status -- nonzero if error */
	register PyObject *x;	/* Result object -- NULL if error */
	register PyObject *v;	/* Temporary objects popped off stack */
	register PyObject *w;
	register PyObject **fastlocals, **freevars;
	PyObject *retval = NULL;	/* Return value */
	PyThreadState *tstate = PyThreadState_GET();
	PyCodeObject *co;

	unsigned char *first_instr;
	PyObject *names;
	PyObject *consts;

/* Tuple access macros */

#define GETITEM(v, i) PyTuple_GET_ITEM((PyTupleObject *)(v), (i))

/* Code access macros */

#define INSTR_OFFSET()	(next_instr - first_instr)
#define NEXTOP()	(*next_instr++)
#define NEXTARG()	(next_instr += 2, (next_instr[-1]<<8) + next_instr[-2])
#define JUMPTO(x)	(next_instr = first_instr + (x))
#define JUMPBY(x)	(next_instr += (x))

/* OpCode prediction macros
	Some opcodes tend to come in pairs thus making it possible to predict
	the second code when the first is run.  For example, COMPARE_OP is often
	followed by JUMP_IF_FALSE or JUMP_IF_TRUE.  And, those opcodes are often
	followed by a POP_TOP.

	Verifying the prediction costs a single high-speed test of register
	variable against a constant.  If the pairing was good, then the
	processor has a high likelihood of making its own successful branch
	prediction which results in a nearly zero overhead transition to the
	next opcode.

	A successful prediction saves a trip through the eval-loop including
	its two unpredictable branches, the HASARG test and the switch-case.
*/

#define PREDICT(op)		if (*next_instr == op) goto PRED_##op
#define PREDICTED(op)		PRED_##op: next_instr++
#define PREDICTED_WITH_ARG(op)	PRED_##op: oparg = (next_instr[2]<<8) + \
				next_instr[1]; next_instr += 3

/* Stack manipulation macros */

#define STACK_LEVEL()	(stack_pointer - f->f_valuestack)
#define EMPTY()		(STACK_LEVEL() == 0)
#define TOP()		(stack_pointer[-1])
#define SECOND()	(stack_pointer[-2])
#define THIRD() 	(stack_pointer[-3])
#define FOURTH()	(stack_pointer[-4])
#define SET_TOP(v)	(stack_pointer[-1] = (v))
#define SET_SECOND(v)	(stack_pointer[-2] = (v))
#define SET_THIRD(v)	(stack_pointer[-3] = (v))
#define SET_FOURTH(v)	(stack_pointer[-4] = (v))
#define BASIC_STACKADJ(n)	(stack_pointer += n)
#define BASIC_PUSH(v)	(*stack_pointer++ = (v))
#define BASIC_POP()	(*--stack_pointer)

#define PUSH(v)		BASIC_PUSH(v)
#define POP()		BASIC_POP()
#define STACKADJ(n)	BASIC_STACKADJ(n)

/* Local variable macros */

#define GETLOCAL(i)	(fastlocals[i])

/* The SETLOCAL() macro must not DECREF the local variable in-place and
   then store the new value; it must copy the old value to a temporary
   value, then store the new value, and then DECREF the temporary value.
   This is because it is possible that during the DECREF the frame is
   accessed by other code (e.g. a __del__ method or gc.collect()) and the
   variable would be pointing to already-freed memory. */
#define SETLOCAL(i, value)	do { PyObject *tmp = GETLOCAL(i); \
				     GETLOCAL(i) = value; \
                                     Py_XDECREF(tmp); } while (0)

/* Start of code */

	if (f == NULL)
		return NULL;

	/* push frame */
	if (++tstate->recursion_depth > recursion_limit) {
		--tstate->recursion_depth;
		/* ERROR */
		tstate->frame = f->f_back;
		return NULL;
	}

	tstate->frame = f;

	/* tracing elided */

	co = f->f_code;
	names = co->co_names;
	consts = co->co_consts;
	fastlocals = f->f_localsplus;
	freevars = f->f_localsplus + f->f_nlocals;

	_PyCode_GETCODEPTR(co, &first_instr);

	/* An explanation is in order for the next line.

	   f->f_lasti now refers to the index of the last instruction
	   executed.  You might think this was obvious from the name, but
	   this wasn't always true before 2.3!  PyFrame_New now sets
	   f->f_lasti to -1 (i.e. the index *before* the first instruction)
	   and YIELD_VALUE doesn't fiddle with f_lasti any more.  So this
	   does work.  Promise. */
	next_instr = first_instr + f->f_lasti + 1;
	stack_pointer = f->f_stacktop;

	f->f_stacktop = NULL;	/* remains NULL unless yield suspends frame */

	why = WHY_NOT;
	err = 0;
	x = Py_None;	/* Not a reference, just anything non-NULL */
	w = NULL;

	for (;;) {

	  /* @@@ pending calls elided */

	fast_next_opcode:
		f->f_lasti = INSTR_OFFSET();

		/* Extract opcode and argument */

		opcode = NEXTOP();
		if (HAS_ARG(opcode))
			oparg = NEXTARG();

		/* Main switch on opcode */

		switch (opcode) {

		/* BEWARE!
		   It is essential that any operation that fails sets either
		   x to NULL, err to nonzero, or why to anything but WHY_NOT,
		   and that no operation that succeeds does this! */

		/* case STOP_CODE: this is an error! */

		case LOAD_CONST:
			x = GETITEM(consts, oparg);
			Py_INCREF(x);
			PUSH(x);
			goto fast_next_opcode;

		case BINARY_ADD:
			w = POP();
			v = TOP();
			if (PyInt_CheckExact(v) && PyInt_CheckExact(w)) {
				/* INLINE: int + int */
				register long a, b, i;
				a = PyInt_AS_LONG(v);
				b = PyInt_AS_LONG(w);
				i = a + b;
				if ((i^a) < 0 && (i^b) < 0)
					goto slow_add;
				x = PyInt_FromLong(i);
			}
			else {
			  slow_add:
				x = PyNumber_Add(v, w);
			}
			Py_DECREF(v);
			Py_DECREF(w);
			SET_TOP(x);
			if (x != NULL) continue;
			break;

		case PRINT_ITEM:
			v = POP();

			PyObject_Print(v);
			Py_DECREF(v);
			break;

		default:
		  Py_FatalError("unknown opcode");
		} /* switch */

		if (why == WHY_NOT) {
			if (err == 0 && x != NULL) {
					continue; /* Normal, fast path */
			}
			why = WHY_EXCEPTION;
			x = Py_None;
			err = 0;
		}

		/* End the loop if we still have an error (or return) */

		if (why != WHY_NOT)
			break;

	} /* main loop */

	if (why != WHY_YIELD) {
		/* Pop remaining stack entries -- but when yielding */
		while (!EMPTY()) {
			v = POP();
			Py_XDECREF(v);
		}
	}

	if (why != WHY_RETURN && why != WHY_YIELD)
		retval = NULL;

	/* pop frame */
	--tstate->recursion_depth;
	tstate->frame = f->f_back;

	return retval;
}

PyObject *
PyEval_EvalCodeEx(PyCodeObject *co, PyObject *globals, PyObject *locals,
	   PyObject **args, int argcount, PyObject **kws, int kwcount,
	   PyObject **defs, int defcount, PyObject *closure)
{
  printf("> PyEval_EvalCodeEx\n");

	register PyFrameObject *f;
	register PyObject *retval = NULL;
	register PyObject **fastlocals, **freevars;
	PyThreadState *tstate = PyThreadState_GET();
	PyObject *x, *u;

	if (globals == NULL) {
	  /* ERROR */
		return NULL;
	}

	f = PyFrame_New(tstate, co, globals, locals);
	if (f == NULL)
		return NULL;

	fastlocals = f->f_localsplus;
	freevars = f->f_localsplus + f->f_nlocals;

	if (co->co_argcount > 0 ||
	    co->co_flags & (CO_VARARGS | CO_VARKEYWORDS)) {
		int i;
		int n = argcount;
		PyObject *kwdict = NULL;
		if (co->co_flags & CO_VARKEYWORDS) {
			kwdict = PyDict_New();
			if (kwdict == NULL)
				goto fail;
			i = co->co_argcount;
			if (co->co_flags & CO_VARARGS)
				i++;
			SETLOCAL(i, kwdict);
		}
		if (argcount > co->co_argcount) {
			if (!(co->co_flags & CO_VARARGS)) {
			  /* ERROR */
				goto fail;
			}
			n = co->co_argcount;
		}
		for (i = 0; i < n; i++) {
			x = args[i];
			Py_INCREF(x);
			SETLOCAL(i, x);
		}
		if (co->co_flags & CO_VARARGS) {
			u = PyTuple_New(argcount - n);
			if (u == NULL)
				goto fail;
			SETLOCAL(co->co_argcount, u);
			for (i = n; i < argcount; i++) {
				x = args[i];
				Py_INCREF(x);
				PyTuple_SET_ITEM(u, i-n, x);
			}
		}
		for (i = 0; i < kwcount; i++) {
			PyObject *keyword = kws[2*i];
			PyObject *value = kws[2*i + 1];
			int j;
			if (keyword == NULL || !PyString_Check(keyword)) {
			  /* ERROR */
				goto fail;
			}
			/* XXX slow -- speed up using dictionary? */
			for (j = 0; j < co->co_argcount; j++) {
				PyObject *nm = PyTuple_GET_ITEM(
					co->co_varnames, j);
				int cmp = PyObject_RichCompareBool(
					keyword, nm, Py_EQ);
				if (cmp > 0)
					break;
				else if (cmp < 0)
					goto fail;
			}
			/* Check errors from Compare */
			/* ERROR */
			if (j >= co->co_argcount) {
				if (kwdict == NULL) {
				  /* ERROR */
					goto fail;
				}
				PyDict_SetItem(kwdict, keyword, value);
			}
			else {
				if (GETLOCAL(j) != NULL) {
				  /* ERROR */
					goto fail;
				}
				Py_INCREF(value);
				SETLOCAL(j, value);
			}
		}
		if (argcount < co->co_argcount) {
			int m = co->co_argcount - defcount;
			for (i = argcount; i < m; i++) {
				if (GETLOCAL(i) == NULL) {
				  /* ERROR */
					goto fail;
				}
			}
			if (n > m)
				i = n - m;
			else
				i = 0;
			for (; i < defcount; i++) {
				if (GETLOCAL(m+i) == NULL) {
					PyObject *def = defs[i];
					Py_INCREF(def);
					SETLOCAL(m+i, def);
				}
			}
		}
	}
	else {
		if (argcount > 0 || kwcount > 0) {
		  /* ERROR */
			goto fail;
		}
	}

	/* Allocate and initialize storage for cell vars, and copy free
	   vars into frame.  This isn't too efficient right now. */
	if (f->f_ncells) {
		int i = 0, j = 0, nargs, found;
		char *cellname, *argname;
		PyObject *c;

		nargs = co->co_argcount;
		if (co->co_flags & CO_VARARGS)
			nargs++;
		if (co->co_flags & CO_VARKEYWORDS)
			nargs++;

		/* Check for cells that shadow args */
		for (i = 0; i < f->f_ncells && j < nargs; ++i) {
			cellname = PyString_AS_STRING(
				PyTuple_GET_ITEM(co->co_cellvars, i));
			found = 0;
			while (j < nargs) {
				argname = PyString_AS_STRING(
					PyTuple_GET_ITEM(co->co_varnames, j));
				if (strcmp(cellname, argname) == 0) {
					c = PyCell_New(GETLOCAL(j));
					if (c == NULL)
						goto fail;
					GETLOCAL(f->f_nlocals + i) = c;
					found = 1;
					break;
				}
				j++;
			}
			if (found == 0) {
				c = PyCell_New(NULL);
				if (c == NULL)
					goto fail;
				SETLOCAL(f->f_nlocals + i, c);
			}
		}
		/* Initialize any that are left */
		while (i < f->f_ncells) {
			c = PyCell_New(NULL);
			if (c == NULL)
				goto fail;
			SETLOCAL(f->f_nlocals + i, c);
			i++;
		}
	}

	if (f->f_nfreevars) {
		int i;
		for (i = 0; i < f->f_nfreevars; ++i) {
			PyObject *o = PyTuple_GET_ITEM(closure, i);
			Py_INCREF(o);
			freevars[f->f_ncells + i] = o;
		}
	}

	/* generator support elided */

        retval = eval_frame(f);

  fail: /* Jump here from prelude on failure */

	/* decref'ing the frame can cause __del__ methods to get invoked,
	   which can call back into Python.  While we're done with the
	   current Python frame (f), the associated C stack is still in use,
	   so recursion_depth must be boosted for the duration.
	*/
	++tstate->recursion_depth;
        Py_DECREF(f);
	--tstate->recursion_depth;
	printf("< PyEval_EvalCodeEx\n");
	return retval;
}
