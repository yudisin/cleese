
/* Python interpreter top-level routines, including init/exit */

#include "Python.h"

#include "compile.h"
#include "marshal.h"
#include "eval.h"

#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif

/* Forward */
static void initmain(void);
static void initsite(void);
static void initsigs(void);

static PyObject *run_pyc_file(FILE *, const char *, PyObject *, PyObject *, PyCompilerFlags *);

static void call_sys_exitfunc(void);
static void call_ll_exitfuncs(void);
extern void _PyUnicode_Init(void);
extern void _PyUnicode_Fini(void);

#ifdef WITH_THREAD
extern void _PyGILState_Init(PyInterpreterState *, PyThreadState *);
extern void _PyGILState_Fini(void);
#endif /* WITH_THREAD */

int Py_NoSiteFlag = 0;
int Py_IgnoreEnvironmentFlag = 0;
int Py_VerboseFlag = 0;
int Py_UnicodeFlag = 0;
int Py_FrozenFlag = 0;
int _Py_QnewFlag = 0;


/* Reference to 'warnings' module, to avoid importing it
   on the fly when the import lock may be held.  See 683658/771097
*/
static PyObject *warnings_module = NULL;

/* Returns a borrowed reference to the 'warnings' module, or NULL.
   If the module is returned, it is guaranteed to have been obtained
   without acquiring the import lock
*/
PyObject *PyModule_GetWarningsModule()
{
        PyObject *typ, *val, *tb;
        PyObject *all_modules;
        /* If we managed to get the module at init time, just use it */
        if (warnings_module)
                return warnings_module;
        /* If it wasn't available at init time, it may be available
           now in sys.modules (common scenario is frozen apps: import
           at init time fails, but the frozen init code sets up sys.path
           correctly, then does an implicit import of warnings for us
        */
        /* Save and restore any exceptions */
        PyErr_Fetch(&typ, &val, &tb);

        all_modules = PySys_GetObject("modules");
        if (all_modules) {
                warnings_module = PyDict_GetItemString(all_modules, "warnings");
                /* We keep a ref in the global */
                Py_XINCREF(warnings_module);
        }
        PyErr_Restore(typ, val, tb);
        return warnings_module;
}

static int initialized = 0;

/* API to access the initialized flag -- useful for esoteric use */

int
Py_IsInitialized(void)
{
	return initialized;
}

/* Global initializations.  Can be undone by Py_Finalize().  Don't
   call this twice without an intervening Py_Finalize() call.  When
   initializations fail, a fatal error is issued and the function does
   not return.  On return, the first thread and interpreter state have
   been created.

   Locking: you must hold the interpreter lock while calling this.
   (If the lock has not yet been initialized, that's equivalent to
   having the lock, but you cannot use multiple threads.)

*/

void
Py_Initialize(void)
{
	PyInterpreterState *interp;
	PyThreadState *tstate;
	PyObject *bimod, *sysmod;
	extern void _Py_ReadyTypes(void);

	if (initialized)
		return;
	initialized = 1;

	interp = PyInterpreterState_New();
	if (interp == NULL)
		Py_FatalError("Py_Initialize: can't make first interpreter");

	tstate = PyThreadState_New(interp);
	if (tstate == NULL)
		Py_FatalError("Py_Initialize: can't make first thread");
	(void) PyThreadState_Swap(tstate);

	_Py_ReadyTypes();

	if (!_PyFrame_Init())
		Py_FatalError("Py_Initialize: can't init frames");

	if (!_PyInt_Init())
		Py_FatalError("Py_Initialize: can't init ints");

	interp->modules = PyDict_New();
	if (interp->modules == NULL)
		Py_FatalError("Py_Initialize: can't make modules dictionary");

#ifdef Py_USING_UNICODE
	/* Init Unicode implementation; relies on the codec registry */
	_PyUnicode_Init();
#endif

	bimod = _PyBuiltin_Init();
	if (bimod == NULL)
		Py_FatalError("Py_Initialize: can't initialize __builtin__");
	interp->builtins = PyModule_GetDict(bimod);
	Py_INCREF(interp->builtins);

	sysmod = _PySys_Init();
	if (sysmod == NULL)
		Py_FatalError("Py_Initialize: can't initialize sys");
	interp->sysdict = PyModule_GetDict(sysmod);
	Py_INCREF(interp->sysdict);
	_PyImport_FixupExtension("sys", "sys");
	PySys_SetPath(Py_GetPath());
	PyDict_SetItemString(interp->sysdict, "modules",
			     interp->modules);

	_PyImport_Init();

	/* initialize builtin exceptions */
	_PyExc_Init();
	_PyImport_FixupExtension("exceptions", "exceptions");

	/* phase 2 of builtins */
	_PyImport_FixupExtension("__builtin__", "__builtin__");

	_PyImportHooks_Init();

	initsigs(); /* Signal handling stuff, including initintr() */

	initmain(); /* Module __main__ */
	if (!Py_NoSiteFlag)
		initsite(); /* Module site */

	/* auto-thread-state API, if available */
#ifdef WITH_THREAD
	_PyGILState_Init(interp, tstate);
#endif /* WITH_THREAD */

	warnings_module = PyImport_ImportModule("warnings");
	if (!warnings_module)
		PyErr_Clear();

#if defined(Py_USING_UNICODE) && defined(HAVE_LANGINFO_H) && defined(CODESET)
	/* On Unix, set the file system encoding according to the
	   user's preference, if the CODESET names a well-known
	   Python codec, and Py_FileSystemDefaultEncoding isn't
	   initialized by other means.  */
	if (!Py_FileSystemDefaultEncoding) {
		char *saved_locale = setlocale(LC_CTYPE, NULL);
		char *codeset;
		setlocale(LC_CTYPE, "");
		codeset = nl_langinfo(CODESET);
		if (*codeset) {
			PyObject *enc = PyCodec_Encoder(codeset);
			if (enc) {
				Py_FileSystemDefaultEncoding = strdup(codeset);
				Py_DECREF(enc);
			} else
				PyErr_Clear();
		}
		setlocale(LC_CTYPE, saved_locale);
	}
#endif
}

#ifdef COUNT_ALLOCS
extern void dump_counts(void);
#endif

/* Undo the effect of Py_Initialize().

   Beware: if multiple interpreter and/or thread states exist, these
   are not wiped out; only the current thread and interpreter state
   are deleted.  But since everything else is deleted, those other
   interpreter and thread states should no longer be used.

   (XXX We should do better, e.g. wipe out all interpreters and
   threads.)

   Locking: as above.

*/

void
Py_Finalize(void)
{
	PyInterpreterState *interp;
	PyThreadState *tstate;

	if (!initialized)
		return;

	/* The interpreter is still entirely intact at this point, and the
	 * exit funcs may be relying on that.  In particular, if some thread
	 * or exit func is still waiting to do an import, the import machinery
	 * expects Py_IsInitialized() to return true.  So don't say the
	 * interpreter is uninitialized until after the exit funcs have run.
	 * Note that Threading.py uses an exit func to do a join on all the
	 * threads created thru it, so this also protects pending imports in
	 * the threads created via Threading.
	 */
	call_sys_exitfunc();
	initialized = 0;

	/* Get current thread state and interpreter pointer */
	tstate = PyThreadState_Get();
	interp = tstate->interp;

	/* Disable signal handling */
	PyOS_FiniInterrupts();

	/* drop module references we saved */
	Py_XDECREF(warnings_module);
	warnings_module = NULL;

	/* Collect garbage.  This may call finalizers; it's nice to call these
	   before all modules are destroyed. */
	PyGC_Collect();

	/* Destroy all modules */
	PyImport_Cleanup();

	/* Collect final garbage.  This disposes of cycles created by
	   new-style class definitions, for example. */
	PyGC_Collect();

	/* Destroy the database used by _PyImport_{Fixup,Find}Extension */
	_PyImport_Fini();

	/* Debugging stuff */
#ifdef COUNT_ALLOCS
	dump_counts();
#endif

#ifdef Py_REF_DEBUG
	fprintf(stderr, "[%ld refs]\n", _Py_RefTotal);
#endif

#ifdef Py_TRACE_REFS
	/* Display all objects still alive -- this can invoke arbitrary
	 * __repr__ overrides, so requires a mostly-intact interpreter.
	 * Alas, a lot of stuff may still be alive now that will be cleaned
	 * up later.
	 */
	if (Py_GETENV("PYTHONDUMPREFS"))
		_Py_PrintReferences(stderr);
#endif /* Py_TRACE_REFS */

	/* Now we decref the exception classes.  After this point nothing
	   can raise an exception.  That's okay, because each Fini() method
	   below has been checked to make sure no exceptions are ever
	   raised.
	*/
	_PyExc_Fini();

	/* Cleanup auto-thread-state */
#ifdef WITH_THREAD
	_PyGILState_Fini();
#endif /* WITH_THREAD */

	/* Clear interpreter state */
	PyInterpreterState_Clear(interp);

	/* Delete current thread */
	PyThreadState_Swap(NULL);
	PyInterpreterState_Delete(interp);

	/* Sundry finalizers */
	PyMethod_Fini();
	PyFrame_Fini();
	PyCFunction_Fini();
	PyTuple_Fini();
	PyString_Fini();
	PyInt_Fini();
	PyFloat_Fini();

#ifdef Py_USING_UNICODE
	/* Cleanup Unicode implementation */
	_PyUnicode_Fini();
#endif

	call_ll_exitfuncs();
}

/* Create and initialize a new interpreter and thread, and return the
   new thread.  This requires that Py_Initialize() has been called
   first.

   Unsuccessful initialization yields a NULL pointer.  Note that *no*
   exception information is available even in this case -- the
   exception information is held in the thread, and there is no
   thread.

   Locking: as above.

*/

PyThreadState *
Py_NewInterpreter(void)
{
	PyInterpreterState *interp;
	PyThreadState *tstate, *save_tstate;
	PyObject *bimod, *sysmod;

	if (!initialized)
		Py_FatalError("Py_NewInterpreter: call Py_Initialize first");

	interp = PyInterpreterState_New();
	if (interp == NULL)
		return NULL;

	tstate = PyThreadState_New(interp);
	if (tstate == NULL) {
		PyInterpreterState_Delete(interp);
		return NULL;
	}

	save_tstate = PyThreadState_Swap(tstate);

	/* XXX The following is lax in error checking */

	interp->modules = PyDict_New();

	bimod = _PyImport_FindExtension("__builtin__", "__builtin__");
	if (bimod != NULL) {
		interp->builtins = PyModule_GetDict(bimod);
		Py_INCREF(interp->builtins);
	}
	sysmod = _PyImport_FindExtension("sys", "sys");
	if (bimod != NULL && sysmod != NULL) {
		interp->sysdict = PyModule_GetDict(sysmod);
		Py_INCREF(interp->sysdict);
		PySys_SetPath(Py_GetPath());
		PyDict_SetItemString(interp->sysdict, "modules",
				     interp->modules);
		_PyImportHooks_Init();
		initmain();
		if (!Py_NoSiteFlag)
			initsite();
	}

	if (!PyErr_Occurred())
		return tstate;

	/* Oops, it didn't work.  Undo it all. */

	PyErr_Print();
	PyThreadState_Clear(tstate);
	PyThreadState_Swap(save_tstate);
	PyThreadState_Delete(tstate);
	PyInterpreterState_Delete(interp);

	return NULL;
}

/* Delete an interpreter and its last thread.  This requires that the
   given thread state is current, that the thread has no remaining
   frames, and that it is its interpreter's only remaining thread.
   It is a fatal error to violate these constraints.

   (Py_Finalize() doesn't have these constraints -- it zaps
   everything, regardless.)

   Locking: as above.

*/

void
Py_EndInterpreter(PyThreadState *tstate)
{
	PyInterpreterState *interp = tstate->interp;

	if (tstate != PyThreadState_Get())
		Py_FatalError("Py_EndInterpreter: thread is not current");
	if (tstate->frame != NULL)
		Py_FatalError("Py_EndInterpreter: thread still has a frame");
	if (tstate != interp->tstate_head || tstate->next != NULL)
		Py_FatalError("Py_EndInterpreter: not the last thread");

	PyImport_Cleanup();
	PyInterpreterState_Clear(interp);
	PyThreadState_Swap(NULL);
	PyInterpreterState_Delete(interp);
}

static char *progname = "python";

char *
Py_GetProgramName(void)
{
	return progname;
}

static char *default_home = NULL;

void
Py_SetPythonHome(char *home)
{
	default_home = home;
}

char *
Py_GetPythonHome(void)
{
	char *home = default_home;
	if (home == NULL && !Py_IgnoreEnvironmentFlag)
		home = Py_GETENV("PYTHONHOME");
	return home;
}

/* Create __main__ module */

static void
initmain(void)
{
	PyObject *m, *d;
	m = PyImport_AddModule("__main__");
	if (m == NULL)
		Py_FatalError("can't create __main__ module");
	d = PyModule_GetDict(m);
	if (PyDict_GetItemString(d, "__builtins__") == NULL) {
		PyObject *bimod = PyImport_ImportModule("__builtin__");
		if (bimod == NULL ||
		    PyDict_SetItemString(d, "__builtins__", bimod) != 0)
			Py_FatalError("can't add __builtins__ to __main__");
		Py_DECREF(bimod);
	}
}

/* Import the site module (not into __main__ though) */

static void
initsite(void)
{
	PyObject *m, *f;
	m = PyImport_ImportModule("site");
	if (m == NULL) {
		f = PySys_GetObject("stderr");
		if (Py_VerboseFlag) {
			PyFile_WriteString(
				"'import site' failed; traceback:\n", f);
			PyErr_Print();
		}
		else {
			PyFile_WriteString(
			  "'import site' failed; use -v for traceback\n", f);
			PyErr_Clear();
		}
	}
	else {
		Py_DECREF(m);
	}
}

int
PyRun_SimpleFile(FILE *fp, const char *filename)
{
	PyObject *m, *d, *v;

	m = PyImport_AddModule("__main__");
	if (m == NULL)
		return -1;
	d = PyModule_GetDict(m);
	if (PyDict_GetItemString(d, "__file__") == NULL) {
		PyObject *f = PyString_FromString(filename);
		if (f == NULL)
			return -1;
		if (PyDict_SetItemString(d, "__file__", f) < 0) {
			Py_DECREF(f);
			return -1;
		}
		Py_DECREF(f);
	}

	v = run_pyc_file(fp, filename, d, d, NULL);

	if (v == NULL) {
		PyErr_Print();
		return -1;
	}
	Py_DECREF(v);
	if (Py_FlushLine())
		PyErr_Clear();
	return 0;
}

static int
parse_syntax_error(PyObject *err, PyObject **message, const char **filename,
		   int *lineno, int *offset, const char **text)
{
	long hold;
	PyObject *v;

	/* old style errors */
	if (PyTuple_Check(err))
		return PyArg_ParseTuple(err, "O(ziiz)", message, filename,
				        lineno, offset, text);

	/* new style errors.  `err' is an instance */

	if (! (v = PyObject_GetAttrString(err, "msg")))
		goto finally;
	*message = v;

	if (!(v = PyObject_GetAttrString(err, "filename")))
		goto finally;
	if (v == Py_None)
		*filename = NULL;
	else if (! (*filename = PyString_AsString(v)))
		goto finally;

	Py_DECREF(v);
	if (!(v = PyObject_GetAttrString(err, "lineno")))
		goto finally;
	hold = PyInt_AsLong(v);
	Py_DECREF(v);
	v = NULL;
	if (hold < 0 && PyErr_Occurred())
		goto finally;
	*lineno = (int)hold;

	if (!(v = PyObject_GetAttrString(err, "offset")))
		goto finally;
	if (v == Py_None) {
		*offset = -1;
		Py_DECREF(v);
		v = NULL;
	} else {
		hold = PyInt_AsLong(v);
		Py_DECREF(v);
		v = NULL;
		if (hold < 0 && PyErr_Occurred())
			goto finally;
		*offset = (int)hold;
	}

	if (!(v = PyObject_GetAttrString(err, "text")))
		goto finally;
	if (v == Py_None)
		*text = NULL;
	else if (! (*text = PyString_AsString(v)))
		goto finally;
	Py_DECREF(v);
	return 1;

finally:
	Py_XDECREF(v);
	return 0;
}

void
PyErr_Print(void)
{
	PyErr_PrintEx(1);
}

static void
print_error_text(PyObject *f, int offset, const char *text)
{
	char *nl;
	if (offset >= 0) {
		if (offset > 0 && offset == (int)strlen(text))
			offset--;
		for (;;) {
			nl = strchr(text, '\n');
			if (nl == NULL || nl-text >= offset)
				break;
			offset -= (nl+1-text);
			text = nl+1;
		}
		while (*text == ' ' || *text == '\t') {
			text++;
			offset--;
		}
	}
	PyFile_WriteString("    ", f);
	PyFile_WriteString(text, f);
	if (*text == '\0' || text[strlen(text)-1] != '\n')
		PyFile_WriteString("\n", f);
	if (offset == -1)
		return;
	PyFile_WriteString("    ", f);
	offset--;
	while (offset > 0) {
		PyFile_WriteString(" ", f);
		offset--;
	}
	PyFile_WriteString("^\n", f);
}

static void
handle_system_exit(void)
{
        PyObject *exception, *value, *tb;
        int exitcode = 0;

	PyErr_Fetch(&exception, &value, &tb);
	if (Py_FlushLine())
		PyErr_Clear();
	fflush(stdout);
	if (value == NULL || value == Py_None)
		goto done;
	if (PyInstance_Check(value)) {
		/* The error code should be in the `code' attribute. */
		PyObject *code = PyObject_GetAttrString(value, "code");
		if (code) {
			Py_DECREF(value);
			value = code;
			if (value == Py_None)
				goto done;
		}
		/* If we failed to dig out the 'code' attribute,
		   just let the else clause below print the error. */
	}
	if (PyInt_Check(value))
		exitcode = (int)PyInt_AsLong(value);
	else {
		PyObject_Print(value, stderr, Py_PRINT_RAW);
		PySys_WriteStderr("\n");
		exitcode = 1;
	}
 done:
 	/* Restore and clear the exception info, in order to properly decref
 	 * the exception, value, and traceback.  If we just exit instead,
 	 * these leak, which confuses PYTHONDUMPREFS output, and may prevent
 	 * some finalizers from running.
 	 */
	PyErr_Restore(exception, value, tb);
	PyErr_Clear();
	Py_Exit(exitcode);
	/* NOTREACHED */
}

void
PyErr_PrintEx(int set_sys_last_vars)
{
	PyObject *exception, *v, *tb, *hook;

	if (PyErr_ExceptionMatches(PyExc_SystemExit)) {
		handle_system_exit();
	}
	PyErr_Fetch(&exception, &v, &tb);
	PyErr_NormalizeException(&exception, &v, &tb);
	if (exception == NULL)
		return;
	if (set_sys_last_vars) {
		PySys_SetObject("last_type", exception);
		PySys_SetObject("last_value", v);
		PySys_SetObject("last_traceback", tb);
	}
	hook = PySys_GetObject("excepthook");
	if (hook) {
		PyObject *args = Py_BuildValue("(OOO)",
                    exception, v ? v : Py_None, tb ? tb : Py_None);
		PyObject *result = PyEval_CallObject(hook, args);
		if (result == NULL) {
			PyObject *exception2, *v2, *tb2;
			if (PyErr_ExceptionMatches(PyExc_SystemExit)) {
				handle_system_exit();
			}
			PyErr_Fetch(&exception2, &v2, &tb2);
			PyErr_NormalizeException(&exception2, &v2, &tb2);
			if (Py_FlushLine())
				PyErr_Clear();
			fflush(stdout);
			PySys_WriteStderr("Error in sys.excepthook:\n");
			PyErr_Display(exception2, v2, tb2);
			PySys_WriteStderr("\nOriginal exception was:\n");
			PyErr_Display(exception, v, tb);
			Py_XDECREF(exception2);
			Py_XDECREF(v2);
			Py_XDECREF(tb2);
		}
		Py_XDECREF(result);
		Py_XDECREF(args);
	} else {
		PySys_WriteStderr("sys.excepthook is missing\n");
		PyErr_Display(exception, v, tb);
	}
	Py_XDECREF(exception);
	Py_XDECREF(v);
	Py_XDECREF(tb);
}

void PyErr_Display(PyObject *exception, PyObject *value, PyObject *tb)
{
	int err = 0;
	PyObject *v = value;
	PyObject *f = PySys_GetObject("stderr");
	if (f == NULL)
		fprintf(stderr, "lost sys.stderr\n");
	else {
		if (Py_FlushLine())
			PyErr_Clear();
		fflush(stdout);
		if (tb && tb != Py_None)
			err = PyTraceBack_Print(tb, f);
		if (err == 0 &&
		    PyObject_HasAttrString(v, "print_file_and_line"))
		{
			PyObject *message;
			const char *filename, *text;
			int lineno, offset;
			if (!parse_syntax_error(v, &message, &filename,
						&lineno, &offset, &text))
				PyErr_Clear();
			else {
				char buf[10];
				PyFile_WriteString("  File \"", f);
				if (filename == NULL)
					PyFile_WriteString("<string>", f);
				else
					PyFile_WriteString(filename, f);
				PyFile_WriteString("\", line ", f);
				PyOS_snprintf(buf, sizeof(buf), "%d", lineno);
				PyFile_WriteString(buf, f);
				PyFile_WriteString("\n", f);
				if (text != NULL)
					print_error_text(f, offset, text);
				v = message;
				/* Can't be bothered to check all those
				   PyFile_WriteString() calls */
				if (PyErr_Occurred())
					err = -1;
			}
		}
		if (err) {
			/* Don't do anything else */
		}
		else if (PyClass_Check(exception)) {
			PyClassObject* exc = (PyClassObject*)exception;
			PyObject* className = exc->cl_name;
			PyObject* moduleName =
			      PyDict_GetItemString(exc->cl_dict, "__module__");

			if (moduleName == NULL)
				err = PyFile_WriteString("<unknown>", f);
			else {
				char* modstr = PyString_AsString(moduleName);
				if (modstr && strcmp(modstr, "exceptions"))
				{
					err = PyFile_WriteString(modstr, f);
					err += PyFile_WriteString(".", f);
				}
			}
			if (err == 0) {
				if (className == NULL)
				      err = PyFile_WriteString("<unknown>", f);
				else
				      err = PyFile_WriteObject(className, f,
							       Py_PRINT_RAW);
			}
		}
		else
			err = PyFile_WriteObject(exception, f, Py_PRINT_RAW);
		if (err == 0) {
			if (v != NULL && v != Py_None) {
				PyObject *s = PyObject_Str(v);
				/* only print colon if the str() of the
				   object is not the empty string
				*/
				if (s == NULL)
					err = -1;
				else if (!PyString_Check(s) ||
					 PyString_GET_SIZE(s) != 0)
					err = PyFile_WriteString(": ", f);
				if (err == 0)
				  err = PyFile_WriteObject(s, f, Py_PRINT_RAW);
				Py_XDECREF(s);
			}
		}
		if (err == 0)
			err = PyFile_WriteString("\n", f);
	}
	/* If an error happened here, don't show it.
	   XXX This is wrong, but too many callers rely on this behavior. */
	if (err != 0)
		PyErr_Clear();
}

static PyObject *
run_pyc_file(FILE *fp, const char *filename, PyObject *globals, PyObject *locals,
	     PyCompilerFlags *flags)
{
	PyCodeObject *co;
	PyObject *v;
	long magic;
	long PyImport_GetMagicNumber(void);

	magic = PyMarshal_ReadLongFromFile(fp);
	if (magic != PyImport_GetMagicNumber()) {
		PyErr_SetString(PyExc_RuntimeError,
			   "Bad magic number in .pyc file");
		return NULL;
	}
	(void) PyMarshal_ReadLongFromFile(fp);
	v = PyMarshal_ReadLastObjectFromFile(fp);
	fclose(fp);
	if (v == NULL || !PyCode_Check(v)) {
		Py_XDECREF(v);
		PyErr_SetString(PyExc_RuntimeError,
			   "Bad code object in .pyc file");
		return NULL;
	}
	co = (PyCodeObject *)v;
	v = PyEval_EvalCode(co, globals, locals);
	if (v && flags)
		flags->cf_flags |= (co->co_flags & PyCF_MASK);
	Py_DECREF(co);
	return v;
}

/* Print fatal error message and abort */

void
Py_FatalError(const char *msg)
{
	fprintf(stderr, "Fatal Python error: %s\n", msg);
#ifdef MS_WINDOWS
	OutputDebugString("Fatal Python error: ");
	OutputDebugString(msg);
	OutputDebugString("\n");
#ifdef _DEBUG
	DebugBreak();
#endif
#endif /* MS_WINDOWS */
	abort();
}

/* Clean up and exit */

#ifdef WITH_THREAD
#include "pythread.h"
int _PyThread_Started = 0; /* Set by threadmodule.c and maybe others */
#endif

#define NEXITFUNCS 32
static void (*exitfuncs[NEXITFUNCS])(void);
static int nexitfuncs = 0;

int Py_AtExit(void (*func)(void))
{
	if (nexitfuncs >= NEXITFUNCS)
		return -1;
	exitfuncs[nexitfuncs++] = func;
	return 0;
}

static void
call_sys_exitfunc(void)
{
	PyObject *exitfunc = PySys_GetObject("exitfunc");

	if (exitfunc) {
		PyObject *res;
		Py_INCREF(exitfunc);
		PySys_SetObject("exitfunc", (PyObject *)NULL);
		res = PyEval_CallObject(exitfunc, (PyObject *)NULL);
		if (res == NULL) {
			if (!PyErr_ExceptionMatches(PyExc_SystemExit)) {
				PySys_WriteStderr("Error in sys.exitfunc:\n");
			}
			PyErr_Print();
		}
		Py_DECREF(exitfunc);
	}

	if (Py_FlushLine())
		PyErr_Clear();
}

static void
call_ll_exitfuncs(void)
{
	while (nexitfuncs > 0)
		(*exitfuncs[--nexitfuncs])();

	fflush(stdout);
	fflush(stderr);
}

void
Py_Exit(int sts)
{
	Py_Finalize();

#ifdef macintosh
	PyMac_Exit(sts);
#else
	exit(sts);
#endif
}

static void
initsigs(void)
{
#ifdef HAVE_SIGNAL_H
#ifdef SIGPIPE
	signal(SIGPIPE, SIG_IGN);
#endif
#ifdef SIGXFZ
	signal(SIGXFZ, SIG_IGN);
#endif
#ifdef SIGXFSZ
	signal(SIGXFSZ, SIG_IGN);
#endif
#endif /* HAVE_SIGNAL_H */
	PyOS_InitInterrupts(); /* May imply initsignal() */
}

#ifdef MPW

/* Check for file descriptor connected to interactive device.
   Pretend that stdin is always interactive, other files never. */

int
isatty(int fd)
{
	return fd == fileno(stdin);
}

#endif


/* Wrappers around sigaction() or signal(). */

PyOS_sighandler_t
PyOS_getsig(int sig)
{
#ifdef HAVE_SIGACTION
	struct sigaction context;
	/* Initialize context.sa_handler to SIG_ERR which makes about as
	 * much sense as anything else.  It should get overwritten if
	 * sigaction actually succeeds and otherwise we avoid an
	 * uninitialized memory read.
	 */
	context.sa_handler = SIG_ERR;
	sigaction(sig, NULL, &context);
	return context.sa_handler;
#else
	PyOS_sighandler_t handler;
	handler = signal(sig, SIG_IGN);
	signal(sig, handler);
	return handler;
#endif
}

PyOS_sighandler_t
PyOS_setsig(int sig, PyOS_sighandler_t handler)
{
#ifdef HAVE_SIGACTION
	struct sigaction context;
	PyOS_sighandler_t oldhandler;
	/* Initialize context.sa_handler to SIG_ERR which makes about as
	 * much sense as anything else.  It should get overwritten if
	 * sigaction actually succeeds and otherwise we avoid an
	 * uninitialized memory read.
	 */
	context.sa_handler = SIG_ERR;
	sigaction(sig, NULL, &context);
	oldhandler = context.sa_handler;
	context.sa_handler = handler;
	sigaction(sig, &context, NULL);
	return oldhandler;
#else
	return signal(sig, handler);
#endif
}
