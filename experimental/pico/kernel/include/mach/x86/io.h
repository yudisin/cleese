/*
 * mach/x86/io.h
 *
 */
#ifndef __MACH_X86_IO_H__
#define __MACH_X86_IO_H__

#include <sys/types.h>

/*
 * inportb()
 *      Get a byte from an I/O port
 */
inline extern uchar_t
inportb(int port)
{
        register uchar_t res;

        __asm__ __volatile__(
                "inb %%dx,%%al\n\t"
                : "=a" (res)
                : "d" (port));
        return(res);
}

/*
 * outportb()
 *      Write a byte to an I/O port
 */
inline extern void
outportb(int port, uchar_t data)
{
        __asm__ __volatile__(
                "outb %%al,%%dx\n\t"
                : /* No output */
                : "a" (data), "d" (port));
}

/*
 * inportw()
 *      Get a word from an I/O port
 */
inline extern ushort_t
inportw(int port)
{
        register ushort_t res;

        __asm__ __volatile__(
                "inw %%dx,%%ax\n\t"
                : "=a" (res)
                : "d" (port));
        return(res);
}

/*
 * outportw()
 *      Write a word to an I/O port
 */
inline extern void
outportw(int port, ushort_t data)
{
        __asm__ __volatile__(
                "outw %%ax,%%dx\n\t"
                : /* No output */
                : "a" (data), "d" (port));
}

#endif /* __MACH_X86_IO_H__ */
