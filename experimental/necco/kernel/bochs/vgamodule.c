
#include "Python.h"

static PyObject *
vga_videomode(PyObject *self, PyObject *args)
{
	PyObject *v = NULL;

	if (!PyArg_UnpackTuple(args, "inb", 1, 1, &v))
		return NULL;
	
	if (!PyInt_CheckExact(v))
		return NULL;

	switch(PyInt_AS_LONG(v))	{
	case 0:	vga80x25(); init_screen();	break;
	case 1: vga640x480x16();		break;
	case 2: vga320x200x256();		break;
	}
	Py_INCREF(Py_True);
	return Py_True;
}

static PyMethodDef vga_methods[] = {
	{"videomode",	vga_videomode,  METH_VARARGS, NULL/*doc*/},
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

	vgadefaultpalette();
	vga80x25();
	init_screen();

	return mod;
}
