import sys
import marshal

if sys.version_info != (2, 3, 0, "final", 0):
    print "Freeze must be run using Python 2.3.0 as marshalling format is not guaranteed to be compatible between versions"
    sys.exit(1)

filename = sys.argv[1]

print "Freezing", filename

infp = file(filename)
source_string = infp.read() + "\n"
infp.close()

code_object = compile(source_string, "<frozen>", "exec")
code_string = marshal.dumps(code_object)

outfp = file("kernel_bytecode.c", "w")

outfp.write("""
/* generated by util/freeze.py from %s */

#include"Python.h"

""" % filename)

outfp.write("unsigned char kernel_bytecode[] = {")

for i in range(0, len(code_string), 16):
    outfp.write("\n\t")
    for c in code_string[i:i+16]:
        outfp.write("%d," % ord(c))
outfp.write("""
};

struct _frozen frozenModules[] = {
\t{"__main__", kernel_bytecode, %d},
\t{0, 0, 0} /* sentinel */
};
""" % len(code_string))

outfp.close()
