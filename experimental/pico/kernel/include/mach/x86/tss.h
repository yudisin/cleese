#ifndef __MACH_X86_TSS_H__
#define __MACH_X86_TSS_H__
/*
 * tss.h
 */
#include <sys/types.h>

/* 
 * Task State Segment 
 */
typedef struct {
  ulong_t	link;
  ulong_t	esp0;
  ulong_t       ss0;
  ulong_t	esp1;
  ulong_t       ss1;
  ulong_t	esp2;
  ulong_t       ss2;
  ulong_t	cr3;

  /* 
   * General processor registers
   */
  ulong_t	eip;
  ulong_t	eflags;
  ulong_t	eax;
  ulong_t       ecx;
  ulong_t       edx;
  ulong_t       ebx;
  ulong_t       esp;
  ulong_t       ebp;
  ulong_t	esi;
  ulong_t       edi;

  /*
   * Segment registers
   */
  ulong_t	es;
  ulong_t       cs;
  ulong_t       ss;
  ulong_t       ds;
  ulong_t       fs;
  ulong_t       gs;

  /*
   * GDT selector for the LDT descriptor
   */
  ulong_t	ldt;

  /*
   * If set, causes a debug trap when the TSS
   * becomes active.
   */
  uint_t        dbg:1;
  uint_t        padding:15;

  /*
   * Offset used to the I/O map
   */
  ushort_t	iomap;
} x86_tss_t;

#endif /* __MACH_X86_TSS_H__ */
