#ifndef Py_OPCODE_H
#define Py_OPCODE_H

#define POP_TOP		1

#define UNARY_NEGATIVE	11
#define UNARY_NOT	12

#define BINARY_MULTIPLY	20
#define BINARY_DIVIDE	21
#define BINARY_MODULO	22
#define BINARY_ADD      23
#define BINARY_SUBTRACT 24
#define BINARY_SUBSCR   25

#define SLICE           30
/* Also uses 31-33 */

#define STORE_SLICE	40

#define STORE_SUBSCR	60

#define BINARY_LSHIFT	62
#define BINARY_RSHIFT	63
#define BINARY_AND	64

#define BINARY_OR       66

#define PRINT_ITEM      71
#define PRINT_NEWLINE   72

#define BREAK_LOOP      80

#define RETURN_VALUE    83

#define POP_BLOCK	87

#define HAVE_ARGUMENT   90

#define STORE_NAME      90
#define STORE_GLOBAL	97	/* "" */

#define LOAD_CONST	100	/* Index in const list */
#define LOAD_NAME	101	/* Index in name list */

#define BUILD_TUPLE	102	/* Number of tuple items */

#define LOAD_ATTR	105	/* Index in name list */
#define COMPARE_OP	106	/* Comparison operator */

#define IMPORT_NAME	107	/* Index in name list */

#define JUMP_FORWARD	110	/* Number of bytes to skip */
#define JUMP_IF_FALSE	111	/* "" */
#define JUMP_IF_TRUE	112	/* "" */

#define JUMP_ABSOLUTE	113	/* Target byte offset from beginning of code */

#define LOAD_GLOBAL	116	/* Index in name list */

#define CONTINUE_LOOP	119	/* Start of loop (absolute) */
#define SETUP_LOOP	120	/* Target address (absolute) */
#define SETUP_EXCEPT	121	/* "" */
#define SETUP_FINALLY	122	/* "" */

#define LOAD_FAST	124	/* Local variable number */
#define STORE_FAST	125	/* Local variable number */

#define SET_LINENO	127

/* CALL_FUNCTION_XXX opcodes defined below depend on this definition */
#define CALL_FUNCTION	131	/* #args + (#kwargs<<8) */
#define MAKE_FUNCTION	132	/* #defaults */

#define HAS_ARG(op) ((op) >= HAVE_ARGUMENT)

/* Comparison operator codes (argument to COMPARE_OP) */
enum cmp_op {LT=Py_LT, LE=Py_LE, EQ=Py_EQ, NE=Py_NE, GT=Py_GT, GE=Py_GE,
	     IN, NOT_IN, IS, IS_NOT, EXC_MATCH, BAD};

#endif /* !Py_OPCODE_H */
