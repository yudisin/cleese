/*
 * mach/x86/pic.h
 *
 */
#ifndef __MACH_X86_PIC_H__
#define __MACH_X86_PIC_H__

#include <sys/types.h>
#include <pic.h>

#define PIC1            0x20
#define PIC2            0xA0
#define PIC1_COMMAND    PIC1
#define PIC1_DATA       (PIC1+1)
#define PIC2_COMMAND    PIC2
#define PIC2_DATA       (PIC2+1)
#define PIC_EOI         0x20

#define PIC1_VECTOR     0x20
#define PIC2_VECTOR     0x28

#define ICW1_ICW4       0x01            /* ICW4 (not) needed */
#define ICW1_SINGLE     0x02            /* Single (cascade) mode */
#define ICW1_INTERVAL4  0x04            /* Call address interval 4 (8) */
#define ICW1_LEVEL      0x08            /* Level triggered (edge) mode */
#define ICW1_INIT       0x10            /* Initialization - required! */

#define ICW4_8086       0x01            /* 8086/88 (MCS-80/85) mode */
#define ICW4_AUTO       0x02            /* Auto (normal) EOI */
#define ICW4_BUF_SLAVE  0x08                    /* Buffered mode/slave */
#define ICW4_BUF_MASTER 0x0C            /* Buffered mode/master */
#define ICW4_SFNM       0x10            /* Special fully nested (not) */

/*
 * Core CPU traps
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
#define T_RESERVED2             19
#define T_RESERVED3             20
#define T_RESERVED4             21
#define T_RESERVED5             22
#define T_RESERVED6             23
#define T_RESERVED7             24
#define T_RESERVED8             25
#define T_RESERVED9             26
#define T_RESERVED10            27
#define T_RESERVED11            29
#define T_RESERVED12            29
#define T_RESERVED13            30
#define T_RESERVED14            31

#define T_EXTERNAL              32
#define T_CORE_INTERRUPTS       16

#define T_SYSCALL              255

/*
 * One of these per kernel
 */
extern pic_device_t x86_pic_device;

#endif  /* __MACH_X86_PIC_H__ */
