#ifndef __MACH_X86_GDT_H__
#define __MACH_X86_GDT_H__

#include <sys/types.h>

#define GDT_SEGMENT_SIZE_16    0x0       /* 16-bit segment       */
#define GDT_SEGMENT_SIZE_32    0x1       /* 32-bit segment       */

#define GDT_ACCESS_TYPE_SYSTEM 0x00      /* system               */
#define GDT_ACCESS_TYPE_USER   0x03      /* user                 */

#define GDT_TYPE_INVALID       0x00      /* Invalid segment      */
#define GDT_TYPE_LDT           0x02      /* LDT                  */
#define GDT_TYPE_CALL_GATE_16  0x04      /* 16-bit call gate     */
#define GDT_TYPE_TASK_GATE     0x05      /* task gate            */
#define GDT_TYPE_TSS           0x09      /* task segment         */
#define GDT_TYPE_CALL_GATE     0x0c      /* call gate            */
#define GDT_TYPE_INTR_GATE     0x0e      /* interrupt gate       */
#define GDT_TYPE_TRAP_GATE     0x0f      /* trap gate            */
#define GDT_TYPE_DATA          0x10      /* data                 */
#define GDT_TYPE_DATAW         0x12      /* data+write           */
#define GDT_TYPE_DATAX         0x14      /* data+execute         */
#define GDT_TYPE_DATAXW        0x16      /* data+execute+write   */
#define GDT_TYPE_CODE          0x18      /* code                 */
#define GDT_TYPE_CODER         0x1a      /* code+read            */
#define GDT_TYPE_CODEC         0x1c      /* code+conforming      */
#define GDT_TYPE_DATACR        0x1e      /* code+conforming+read */

/*
 * Identifiers for the various GDT entries we will use.
 */
#define GDT_NULL   (0 << 3)
#define GDT_KDATA  (1 << 3)
#define GDT_KTEXT  (2 << 3)
#define GDT_BOOT32 (3 << 3)
#define GDT_UDATA  (5 << 3)
#define GDT_UTEXT  (6 << 3)
#define GDT_INDEX(i)  ((i) >> 3)

/*
 * Segment descriptor. Total of 64 bits/8 bytes.
 */
typedef struct {
  uint_t  m_limit0:16;     /* limit 0..15  */
  uint_t  m_base0:16;      /* base  0..15  */
  uint_t  m_base1:8;       /* base  16..23 */
  uint_t  m_type:5;        /* type         */
  uint_t  m_dpl:2;         /* access byte  */
  uint_t  m_present:1;     /* present      */
  uint_t  m_limit1:4;      /* limit 16..19 */
  uint_t  m_pad0:2;        /* pad          */
  uint_t  m_32bits:1;      /* is 32 bits?  */
  uint_t  m_granularity:1; /* granularity  */
  uint_t  m_base2:8;       /* base 24..31  */
} x86_descriptor_t;

/*
 * Gate descriptor. Total of 64 bits/8 bytes.
 */
typedef struct {
  uint_t m_offset0 : 16;   /* offset 0          */
  uint_t m_selector : 16;  /* selector          */
  uint_t m_words : 5;      /* words to copy (0) */
  uint_t m_pad0 : 3;       /* pad               */
  uint_t m_type : 5;       /* type              */
  uint_t m_dpl : 2;        /* access            */
  uint_t m_present : 1;    /* present           */
  uint_t m_offset1 : 16;   /* offset 1          */
} x86_gate_t;

/* 
 * Format of a "pseudo-descriptor", used for loading the IDT and GDT.
 * 8 bytes.
 */
typedef struct {
  ushort_t m_pad;
  ushort_t m_length;
  ulong_t  m_address;
} x86_pseudo_descriptor_t;


static inline  void lgdt(x86_pseudo_descriptor_t *pdesc)
{
  __asm__ __volatile__("lgdt %0" : : "m" (pdesc->m_length));
}

static inline  void lidt(x86_pseudo_descriptor_t *pdesc)
{
  __asm__ __volatile__("lidt %0" : : "m" (pdesc->m_length));
}

static inline  void lldt(unsigned short ldt_selector)
{
  __asm__ __volatile__("lldt %w0" : : "r" (ldt_selector));
}

#endif /* __MACH_X86_GDT_H__ */
