#ifndef Py_OPCODE_H
#define Py_OPCODE_H

#define BINARY_ADD 23

#define PRINT_ITEM 71
#define PRINT_NEWLINE 72

#define RETURN_VALUE 83

#define HAVE_ARGUMENT 90

#define STORE_NAME     90

#define LOAD_CONST	100	/* Index in const list */
#define LOAD_NAME	101	/* Index in name list */

#define JUMP_FORWARD	110	/* Number of bytes to skip */

#define SETUP_LOOP	120	/* Target address (absolute) */

#define LOAD_FAST	124	/* Local variable number */

#define HAS_ARG(op) ((op) >= HAVE_ARGUMENT)

#endif /* !Py_OPCODE_H */
