/*
 * NOTE: Most of this comes from VSTa
 */
	
#define GDT_KDATA  (1 << 3)
#define GDT_KTEXT  (2 << 3)

/*
 * Must match pic.h
 */		
#define T_DIV_ZERO               0
#define T_DEBUG                  1
#define T_NON_MASKABLE_INTR      2
#define T_BREAKPOINT             3
#define T_OVERFLOW               4
#define T_BOUNDS_CHECK           5
#define T_ILLEGAL_OPCODE         6
#define T_MISSING_COPRO          7
#define T_DOUBLE_FAULT           8
#define T_MATH_SEGMENT_OVERFLOW  9
#define T_INVALID_TSS           10
#define T_SEGMENT_MISSING       11
#define T_STACK_FAULT           12
#define T_GENERAL_PROTECTION    13
#define T_PAGE_FAULT            14
#define T_RESERVED1             15
#define T_MATH_ERROR            16
#define T_ALIGNMENT_ERROR       17
#define T_MACHINE_CHECK         18
	
/*
 * Common macros to force segment registers to appropriate values
 */
#define SAVE_SEGS \
        pushw %ds ; \
        pushw %es

#define RESTORE_SEGS \
        popw %es ; \
        popw %ds

#define SET_KSEGS \
        movw $(GDT_KDATA),%ax ; \
        mov %ax,%ds ; \
        mov %ax,%es

        .text
	.globl x86_pic_handle_interrupt
	.globl x86_pic_handle_trap
        .align  4
	
/*
 * refresh_segregs()
 *      Load all segment registers with values to match our GDT
 */
        .globl  x86_refresh_segregs
        .align  4
x86_refresh_segregs:
        /* es/ds/ss get our data segment */
        movw $(GDT_KDATA),%ax
        mov     %ax,%ds
        mov     %ax,%es
        mov     %ax,%ss

        /* Trap stray references */
        xorl    %eax,%eax
        mov     %ax,%fs
        mov     %ax,%gs

        /* Refresh CS */
        pushl   $(GDT_KTEXT)
        pushl   $1f
        lret
1:

        /* All done */
        ret

/*
 * x86_ignore_interrupt()
 *   A fast path for ignoring known stray interrupts
 */
        .globl  x86_ignore_interrupt
        .align  4
x86_ignore_interrupt:
        iret

/*
 * trap_common()
 *      Common code for all traps
 *
 * At this point all the various traps and exceptions have been moulded
 * into a single stack format--a OS-type trap number, an error code (0
 * for those which don't have one), and then the saved registers followed
 * by a trap-type stack frame suitable for iret'ing.
 */
        .align  4

trap_common:
        /*
         * Save the user's registers, ensure that we have the direction
         * flag set appropriately and establish the kernel selectors
         */
        cld
        pushal
        SAVE_SEGS
        SET_KSEGS

        /*
         * Call C-code for our common trap()
         */
        call    x86_pic_handle_trap

/*
 * Templates for entry handling.  IDTERR() is for entries which
 * already have an error code supplied by the i386.  IDT() is for
 * those which don't--we push a dummy 0.
 */
#define IDT(n) \
        .globl  TRAP_##n ; \
        .align  4 ; \
        TRAP_##n##: ; \
        pushl   $0 ; \
        pushl   $(n) ; \
        jmp trap_common

#define IDTERR(n) \
        .globl  TRAP_##n ; \
        .align  4 ; \
        TRAP_##n##: ; \
        pushl   $(n) ; \
        jmp trap_common

/*
 * The vectors we handle
 */
IDT(T_DIV_ZERO)
IDT(T_DEBUG)
IDT(T_NON_MASKABLE_INTR)
IDT(T_BREAKPOINT)
IDT(T_OVERFLOW)
IDT(T_BOUNDS_CHECK)
IDT(T_ILLEGAL_OPCODE)
IDT(T_MISSING_COPRO)
IDTERR(T_DOUBLE_FAULT)
IDTERR(T_MATH_SEGMENT_OVERFLOW)
IDTERR(T_INVALID_TSS)
IDTERR(T_SEGMENT_MISSING)
IDTERR(T_STACK_FAULT)
IDTERR(T_GENERAL_PROTECTION)
IDTERR(T_PAGE_FAULT)
IDTERR(T_RESERVED1)
IDTERR(T_MATH_ERROR)
IDTERR(T_ALIGNMENT_ERROR)
IDTERR(T_MACHINE_CHECK)

/*
 * INTVEC()
 *      Macro to set up trap frame for a hardware interrupt
 *
 * We waste the extra pushl to make it look much like a trap frame.
 * This radically simplifies things and makes it easier on the kernel
 * debugger :-)  We push the IRQ number, not the interrupt vector
 * number as this saves some time in the interrupt handler
 */
#define INTVEC(n) \
        .globl  x86_pic_interrupt##n ; \
        .align  4 ; \
x86_pic_interrupt##n##: ; \
        cld ; \
        pushl   $0 ; \
        pushl   $(n - 32) ; \
        pushal ; \
        SAVE_SEGS ; \
        SET_KSEGS ; \
        call    x86_pic_handle_interrupt ; \
        RESTORE_SEGS ; \
        popal ; \
        addl    $8,%esp ; \
        iret

INTVEC(32); INTVEC(33); INTVEC(34); INTVEC(35)
INTVEC(36); INTVEC(37); INTVEC(38); INTVEC(39)
INTVEC(40); INTVEC(41); INTVEC(42); INTVEC(43)
INTVEC(44); INTVEC(45); INTVEC(46); INTVEC(47)
