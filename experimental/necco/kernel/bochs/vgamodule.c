#include "Python.h"

static PyObject *d;
static PyObject *modes[] = {0,0,0};

static PyObject *
vga_videomode(PyObject *self, PyObject *args)
{
	PyObject *v;
	int n;

	if (!PyArg_UnpackTuple(args, "inb", 1, 1, &v))
		return NULL;
	
	if (!PyInt_CheckExact(v))
		return NULL;

	n = PyInt_AS_LONG(v);

	if(n < 0 || n > 2)
		return NULL;

	Py_DECREF((PyObject *)PyEval_EvalCode(
			PyFunction_GET_CODE(modes[n]), d, d));

	if(!n) init_screen();

	Py_INCREF(Py_True);
	return Py_True;
}

static unsigned char saved[4000];

static PyObject *
vga_savetext(PyObject *self, PyObject *args)
{
	memcpy(saved, (void *)0xb8000, 80*25*2);
	Py_INCREF(Py_True);
	return Py_True;
}

static PyObject *
vga_restoretext(PyObject *self, PyObject *args)
{
	memcpy((void *)0xb8000, saved, 80*25*2);
	Py_INCREF(Py_True);
	return Py_True;
}

static PyMethodDef vga_methods[] = {
	{"videomode",	vga_videomode,  METH_VARARGS, NULL/*doc*/},
	{"savetext",	vga_savetext,	METH_VARARGS, NULL/*doc*/},
	{"restoretext",	vga_restoretext,METH_VARARGS, NULL/*doc*/},
	{NULL,		NULL},
};

extern unsigned char bootscreen[];

PyObject *
_VGA_Init(void)
{
	PyObject *mod, *dict;
	mod = Py_InitModule4("vga", vga_methods,
			     NULL/*doc*/, (PyObject *)NULL,
			     PYTHON_API_VERSION);

	dict = PyModule_GetDict(mod);

	#define SETBUILTIN(NAME, OBJECT) \
	if (PyDict_SetItemString(dict, NAME, (PyObject *)OBJECT) < 0)	\
		return NULL;

	SETBUILTIN("framebuffer",
		PyBuffer_FromReadWriteMemory((void *)0xa0000, 0x10000));
	SETBUILTIN("textbuffer",
		PyBuffer_FromReadWriteMemory((void *)0xb8000, 80*25*2));
	SETBUILTIN("splashscreen",
		PyBuffer_FromMemory(bootscreen, 0x10000));

	{ PyObject *m = PyImport_ImportModuleEx("pyvga");
	d = PyModule_GetDict(m);
	modes[0] = PyDict_GetItemString(d, "set80x25");
	modes[1] = PyDict_GetItemString(d, "set640x480x16");
	modes[2] = PyDict_GetItemString(d, "set320x200x256"); }

	/* vga_videomode(NULL, PyInt_FromLong(2)); */
	vgadefaultpalette();
	/* vga_videomode(NULL, PyInt_FromLong(0)); */
	init_screen();

	return mod;
}
