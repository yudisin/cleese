/*
 *  Copyright (C) 2003 Gavin Thomas Nicol
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * pic.c
 *   Interface to the PC PIC and interrupts.
 */
#include <string.h>
#include <kstdlib.h>
#include <mach/x86/io.h>
#include <mach/x86/x86.h>
#include <mach/x86/multiboot.h>

/*
 * These are all in locore.s
 */
extern void x86_refresh_segregs();
extern void x86_ignore_interrupt(ulong_t);
extern void x86_pic_interrupt32(ulong_t);
extern void x86_pic_interrupt33(ulong_t);
extern void x86_pic_interrupt34(ulong_t);
extern void x86_pic_interrupt35(ulong_t);
extern void x86_pic_interrupt36(ulong_t);
extern void x86_pic_interrupt37(ulong_t);
extern void x86_pic_interrupt38(ulong_t);
extern void x86_pic_interrupt39(ulong_t);
extern void x86_pic_interrupt40(ulong_t);
extern void x86_pic_interrupt41(ulong_t);
extern void x86_pic_interrupt42(ulong_t);
extern void x86_pic_interrupt43(ulong_t);
extern void x86_pic_interrupt44(ulong_t);
extern void x86_pic_interrupt45(ulong_t);
extern void x86_pic_interrupt46(ulong_t);
extern void x86_pic_interrupt47(ulong_t);
extern void TRAP_T_DIV_ZERO(ulong_t);
extern void TRAP_T_DEBUG(ulong_t);
extern void TRAP_T_NON_MASKABLE_INTR(ulong_t);
extern void TRAP_T_BREAKPOINT(ulong_t);
extern void TRAP_T_OVERFLOW(ulong_t);
extern void TRAP_T_BOUNDS_CHECK(ulong_t);
extern void TRAP_T_ILLEGAL_OPCODE(ulong_t);
extern void TRAP_T_MISSING_COPRO(ulong_t);
extern void TRAP_T_DOUBLE_FAULT(ulong_t);
extern void TRAP_T_MATH_SEGMENT_OVERFLOW(ulong_t);
extern void TRAP_T_INVALID_TSS(ulong_t);
extern void TRAP_T_SEGMENT_MISSING(ulong_t);
extern void TRAP_T_STACK_FAULT(ulong_t);
extern void TRAP_T_GENERAL_PROTECTION(ulong_t);
extern void TRAP_T_PAGE_FAULT(ulong_t);
extern void TRAP_T_RESERVED1(ulong_t);
extern void TRAP_T_MATH_ERROR(ulong_t);
extern void TRAP_T_ALIGNMENT_ERROR(ulong_t);
extern void TRAP_T_MACHINE_CHECK(ulong_t);

/*
 * Current IRQ mask
 */
static ushort_t x86_irq_mask = 0xFFFF;

/*
 * The IDT we are using
 */
x86_gate_t x86_global_idt[MAX_IDT];

/*
 * Message associated with well known trap errors
 */
struct x86_trap_table {
    char *m_description;
    int m_number;
    void (*m_handler) (ulong_t);
} x86_pic_trap_table[] = {
    { "Divide by Zero", T_DIV_ZERO, TRAP_T_DIV_ZERO}, 
    { "Debug", T_DEBUG, TRAP_T_DEBUG}, 
    { "Non Maskable Interrupt", T_NON_MASKABLE_INTR, TRAP_T_NON_MASKABLE_INTR}, 
    { "Breakpoint", T_BREAKPOINT, TRAP_T_BREAKPOINT}, 
    { "Interrupt on Overflow", T_OVERFLOW, TRAP_T_OVERFLOW}, 
    { "Array Bounds Error", T_BOUNDS_CHECK, TRAP_T_BOUNDS_CHECK}, 
    { "Illegal Opcode", T_ILLEGAL_OPCODE, TRAP_T_ILLEGAL_OPCODE}, 
    { "Math not available", T_MISSING_COPRO, TRAP_T_MISSING_COPRO}, 
    { "Double Fault", T_DOUBLE_FAULT, TRAP_T_DOUBLE_FAULT}, 
    { "Math segment overflow", T_MATH_SEGMENT_OVERFLOW, TRAP_T_MATH_SEGMENT_OVERFLOW}, 
    { "Invalid TSS", T_INVALID_TSS, TRAP_T_INVALID_TSS}, 
    { "Segment not present", T_SEGMENT_MISSING, TRAP_T_SEGMENT_MISSING}, 
    { "Stack Fault", T_STACK_FAULT, TRAP_T_STACK_FAULT}, 
    { "General Protection Error", T_GENERAL_PROTECTION, TRAP_T_GENERAL_PROTECTION}, 
    { "Page Fault", T_PAGE_FAULT, TRAP_T_PAGE_FAULT}, 
    { "Reserved", T_RESERVED1, TRAP_T_RESERVED1}, 
    { "Math Error", T_MATH_ERROR, TRAP_T_MATH_ERROR}, 
    { "Alignment Error", T_ALIGNMENT_ERROR, TRAP_T_ALIGNMENT_ERROR}, 
    { "Machine Check", T_MACHINE_CHECK, TRAP_T_MACHINE_CHECK}, 
    { NULL, 0, NULL},
};

/*
 * This is a table of ISR's associated with a given IRQ.
 * The ISR's are invoked from the commmon interrupt handler.
 */
static interrupt_handler_t x86_pic_isr_table[MAX_IDT];

/*
 * x86_signal_end_of_interrupt()
 *      Clear an interrupt and allow new ones to arrive
 */
static inline void x86_signal_end_of_interrupt(void)
{
    outportb(PIC1_COMMAND, PIC_EOI);
    outportb(PIC2_COMMAND, PIC_EOI);
}

void x86_pic_dump_trap_frame(x86_trap_frame_t * t)
{
    kprintf("  esds:0x%lu\tedi:0x%lu\tesi:0x%lu\n", t->esds, t->edi,
	    t->esi);
    kprintf("  ebp:0x%lu\tESP:0x%lu\tebx:0x%lu\n", t->ebp, t->espdummy,
	    t->ebx);
    kprintf("  edx:0x%lu\tecx:0x%lu\teax:0x%lu\n", t->edx, t->ecx, t->eax);
    kprintf("  eip:0x%lu\tecs:0x%lu\tesp:0x%lu\n", t->eip, t->ecs, t->esp);
    kprintf("  ess:0x%lu\teflags:0x%lu\n", t->ess, t->eflags);
    kprintf("  traptype:0x%lu\terrcode:0x%lu\n", t->traptype, t->errcode);
}

/*
 * Default trap for handling unexpected stuff
 */
void x86_pic_handle_trap(ulong_t p)
{
    x86_trap_frame_t *t = (x86_trap_frame_t *) & p;
    int x;

    kputs("trap:");
    x86_pic_dump_trap_frame(t);

    disable_interrupts();

    x86_irq_mask = 0xFFFF;

    outportb(PIC1_DATA, x86_irq_mask & 0xFF);
    outportb(PIC2_DATA, x86_irq_mask >> 8);

    for (x = 0; x86_pic_trap_table[x].m_description != NULL; x++) {
	if (x86_pic_trap_table[x].m_number == t->errcode) {
	    kprintf("\n%s fault (code %d)\n",
		    x86_pic_trap_table[x].m_description, t->errcode);
	    break;
	}
    }
    kputs("\nFATAL ERROR\n");
    while (1);
}

/*
 * Entry point handling interrupts
 */
void x86_pic_handle_interrupt(ulong_t p)
{
    x86_trap_frame_t *t = (x86_trap_frame_t *) & p;

    /*
     * If the error code is within the scope of our ISR
     * table, try to pick up a handler and invoke it.
     */
    if (t->traptype >= 0 && t->traptype <= MAX_IDT) {
	interrupt_handler_t handler = x86_pic_isr_table[t->traptype];

	if (handler != NULL) {
	    handler(t);
	}
    }
    //x86_pic_dump_trap_frame(t);
    x86_signal_end_of_interrupt();
}

/*
 * Initialize an IDT gate
 */
void x86_pic_setup_gate(int sel, void (*h) (ulong_t), ushort_t type)
{
    ulong_t addr = (ulong_t) h;

    x86_global_idt[sel].m_offset0 = (addr & 0xFFFF);
    x86_global_idt[sel].m_offset1 = (addr >> 16);
    x86_global_idt[sel].m_selector = GDT_KTEXT;
    x86_global_idt[sel].m_type = type;
    x86_global_idt[sel].m_dpl = GDT_ACCESS_TYPE_SYSTEM;
    x86_global_idt[sel].m_words = 0;
    x86_global_idt[sel].m_pad0 = 0;
    x86_global_idt[sel].m_present = 1;
}

/*
 * Initialize the IDT for out system
 */
static void x86_pic_init_idt()
{
    int x;
    int len;
    char *p;
    x86_pseudo_descriptor_t pd;

    kputs("Initializing IDT...\n");

    memset(x86_global_idt, 0x0, MAX_IDT * sizeof(x86_gate_t));
    memset(x86_pic_isr_table, 0x0, MAX_IDT * sizeof(interrupt_handler_t));

    /*
     * First map everything to an ISR that simply ignores everything.
     */
    for (x = 0; x < MAX_IDT; x++) {
	x86_pic_setup_gate(x, x86_ignore_interrupt, GDT_TYPE_TRAP_GATE);
    }

    /*
     * Wire all hard interrupts to a vector which will push their
     * interrupt number and call our common C code.
     */
    p = (char *) x86_pic_interrupt32;
    len = (char *) x86_pic_interrupt33 - (char *) x86_pic_interrupt32;
    for (x = T_EXTERNAL; x < T_EXTERNAL + T_CORE_INTERRUPTS; ++x) {
	x86_pic_setup_gate(x, (void *) (p + (x - T_EXTERNAL) * len),
			   GDT_TYPE_INTR_GATE);
    }

    /*
     * Any other interrupts are stray so handle them appropriately
     */
    for (x = T_EXTERNAL + T_CORE_INTERRUPTS; x < MAX_IDT; ++x) {
	x86_pic_setup_gate(x, x86_ignore_interrupt, GDT_TYPE_INTR_GATE);
    }

    /*
     * Associate handlers with the traps we understand
     */
    for (x = 0; x86_pic_trap_table[x].m_description != NULL; x++) {
	x86_pic_setup_gate(x86_pic_trap_table[x].m_number,
			   x86_pic_trap_table[x].m_handler,
			   GDT_TYPE_TRAP_GATE);
    }

    x86_global_idt[T_PAGE_FAULT].m_type = GDT_TYPE_INTR_GATE;

    /*
     * Load the IDT into hardware
     */
    pd.m_pad = 0;
    pd.m_length = MAX_IDT * sizeof(x86_gate_t);
    pd.m_address = (ulong_t) & x86_global_idt;
    lidt(&pd);

    x86_refresh_segregs();
}

/*
 * Initialize the PIC. We remap the PIC to 0x20 and 0x28.
 */
static void x86_pic_initialize(void *hal, void *ptr)
{
    //multiboot_information_t* mi = (multiboot_information_t*)ptr;
    kputs("Initializing PIC...\n");

    /*
     * We need to remap the PIC's to something reasonable.
     */
    uchar_t a;
    uchar_t b;

    a = inportb(PIC1_DATA);
    b = inportb(PIC2_DATA);

    outportb(PIC1_COMMAND, ICW1_INIT + ICW1_ICW4);
    outportb(PIC2_COMMAND, ICW1_INIT + ICW1_ICW4);
    outportb(PIC1_DATA, PIC1_VECTOR);
    outportb(PIC2_DATA, PIC2_VECTOR);
    outportb(PIC1_DATA, 4);
    outportb(PIC2_DATA, 2);

    outportb(PIC1_DATA, ICW4_8086);
    outportb(PIC2_DATA, ICW4_8086);

    outportb(PIC1_DATA, a);
    outportb(PIC2_DATA, b);

    /*
     * Now wire and load the IDT
     */
    x86_pic_init_idt();
}

/*
 * Shutdown the pic
 */
static void x86_pic_shutdown()
{
    /* Nothing to do here */
}

/*
 * Enable interrupts
 */
static void x86_pic_enable_interrupts()
{
    enable_interrupts();
}

/*
 * Disable interrupts
 */
static void x86_pic_disable_interrupts()
{
    disable_interrupts();
}

/*
 * Enable a given IRQ
 */
static void x86_pic_enable(uchar_t n)
{
    x86_irq_mask &= ~(1 << n);

    if (n < 8) {
	outportb(PIC1_DATA, x86_irq_mask & 0xFF);
    } else {
	outportb(PIC2_DATA, x86_irq_mask >> 8);
    }
}

/*
 * Disable a given IRQ
 */
static void x86_pic_disable(uchar_t n)
{
    x86_irq_mask |= (1 << n);

    if (n < 8) {
	outportb(PIC1_DATA, x86_irq_mask & 0xFF);
    } else {
	outportb(PIC2_DATA, x86_irq_mask >> 8);
    }
}

/*
 * Register a handler for an IRQ
 */
static void x86_pic_register_handler(uchar_t n, interrupt_handler_t h)
{
    x86_pic_isr_table[n] = h;
}

/*
 * Unregister a handler for an IRQ
 */
static void x86_pic_unregister_handler(uchar_t n)
{
    x86_pic_isr_table[n] = NULL;
}

/*
 * External interface to the pic
 */
pic_device_t x86_pic_device = {
    x86_pic_initialize,
    x86_pic_shutdown,
    x86_pic_enable_interrupts,
    x86_pic_disable_interrupts,
    x86_pic_enable,
    x86_pic_disable,
    x86_pic_register_handler,
    x86_pic_unregister_handler,
};
