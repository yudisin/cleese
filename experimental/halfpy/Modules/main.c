/* Python interpreter main program */


#include "Python.h"

/* Main program */

int
Py_Main(int argc, char **argv)
{
	int sts;
	char *filename = NULL;
	FILE *fp = stdin;

	filename = argv[1];

	if (filename != NULL) {
		if ((fp = fopen(filename, "rb")) == NULL) {
			fprintf(stderr, "%s: can't open file '%s'\n",
				argv[0], filename);
			return 2;
		}
	}
	else {
		fprintf(stderr, "no filename to run");
		return 2;
	}

	Py_Initialize();
	sts = PyRun_SimpleFile(fp, filename);
	Py_Finalize();
	return sts;
}
