#ifndef __MACH_X86_ATOMIC_H__
#define __MACH_X86_ATOMIC_H__

/*
 * ATOMIC_INCL()
 *      Increment an integer atomically
 */
inline extern void
ATOMIC_INC(void *val)
{
        __asm__ __volatile__(
                "lock\n\t"
                "incl (%0)\n\t"
                : /* No output */
                : "r" (val));
}

/*
 * ATOMIC_DEC()
 *      Decrement an integer atomically
 */
inline extern void
ATOMIC_DEC(void *val)
{
        __asm__ __volatile__(
                "lock\n\t"
                "decl (%0)\n\t"
                : /* No output */
                : "r" (val));
}

/*
 * ATOMIC_INCL()
 *      Increment a long integer atomically
 */
inline extern void
ATOMIC_INCL(void *val)
{
        __asm__ __volatile__(
                "lock\n\t"
                "incl (%0)\n\t"
                : /* No output */
                : "r" (val));
}

/*
 * ATOMIC_DECL()
 *      Decrement a long integer atomically
 */
inline extern void
ATOMIC_DECL(void *val)
{
        __asm__ __volatile__(
                "lock\n\t"
                "decl (%0)\n\t"
                : /* No output */
                : "r" (val));
}

#endif /* __MACH_X86_ATOMIC_H__ */
