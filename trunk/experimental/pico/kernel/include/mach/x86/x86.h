#ifndef __MACH_X86_X86_H__
#define __MACH_X86_X86_H__

#define MAX_GDT                7
#define MAX_IDT              256

#define MAX_TASKS            1024
#define MAX_TSS              MAX_TASKS

#define PAGE_SIZE            (1024*4)


static inline  void enable_interrupts()
{
  __asm__ __volatile__("sti");
}

static inline  void disable_interrupts()
{
  __asm__ __volatile__("cli");
}

/*
 * This is the shape of the kernel stack after taking an interrupt
 * or exception.  For machine traps which don't provide an error
 * code, we push a 0 ourselves.  "traptype" is from sys/trap.h.
 * edi..eax are in pushal format.
 */
typedef struct {
  ulong_t esds;
  ulong_t edi, esi, ebp, espdummy, ebx, edx, ecx, eax;
  ulong_t traptype;
  ulong_t errcode;
  ulong_t eip, ecs;
  ulong_t eflags;
  ulong_t esp, ess;
} x86_trap_frame_t;

/*
 * Bits in CR0
 */
#define BIT(x) (1 << (x))
#define CR0_PE BIT(0)   /* Protection enable */
#define CR0_MP BIT(1)   /* Math processor present */
#define CR0_EM BIT(2)   /* Emulate FP--trap on FP instruction */
#define CR0_TS BIT(3)   /* Task switched flag */
#define CR0_ET BIT(4)   /* Extension type--387 DX presence */
#define CR0_NE BIT(5)   /* Numeric Error--allow traps on numeric errors */
#define CR0_WP BIT(16)  /* Write protect--ring 0 honors RO PTE's */
#define CR0_AM BIT(18)  /* Alignment--trap on unaligned refs */
#define CR0_NW BIT(29)  /* Not write-through--inhibit write-through */
#define CR0_CD BIT(30)  /* Cache disable */
#define CR0_PG BIT(31)  /* Paging--use PTEs/CR3 */

#include "gdt.h"
#include "pic.h"
#include "tss.h"

#endif /* __MACH_X86_X86_H__ */
