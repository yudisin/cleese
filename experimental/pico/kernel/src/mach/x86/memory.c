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
 * memory.c
 *   Interface to the PC memory
 */
#include <kstdlib.h>
#include <string.h>
#include <mach/x86/io.h>
#include <mach/x86/x86.h>
#include <mach/x86/memory.h>
#include <mach/x86/thread.h>
#include <mach/x86/multiboot.h>

/*
 * This is the GDT that we will switch to once we have booted the
 * system.
 */
x86_descriptor_t x86_global_gdt[MAX_GDT];

/*
 * Craft and load our new GDT
 */
static void x86_init_gdt()
{
    void *ptr;
    x86_pseudo_descriptor_t pd;

    memset(&x86_global_gdt, 0, MAX_GDT * sizeof(x86_descriptor_t));

    /*
     * Kernel data
     * Full 32 bit read+write
     */
    x86_global_gdt[GDT_INDEX(GDT_KDATA)].m_limit0 = 0xFFFF;
    x86_global_gdt[GDT_INDEX(GDT_KDATA)].m_limit1 = 0xF;
    x86_global_gdt[GDT_INDEX(GDT_KDATA)].m_base0 = 0;
    x86_global_gdt[GDT_INDEX(GDT_KDATA)].m_base1 = 0;
    x86_global_gdt[GDT_INDEX(GDT_KDATA)].m_base2 = 0;
    x86_global_gdt[GDT_INDEX(GDT_KDATA)].m_type = GDT_TYPE_DATAW;
    x86_global_gdt[GDT_INDEX(GDT_KDATA)].m_dpl = GDT_ACCESS_TYPE_SYSTEM;
    x86_global_gdt[GDT_INDEX(GDT_KDATA)].m_present = 1;
    x86_global_gdt[GDT_INDEX(GDT_KDATA)].m_32bits = 1;
    x86_global_gdt[GDT_INDEX(GDT_KDATA)].m_granularity = 1;

    /*
     * Kernel text 
     * Low 2G, read+execute
     */
    x86_global_gdt[GDT_INDEX(GDT_KTEXT)].m_limit0 = 0xFFFF;
    x86_global_gdt[GDT_INDEX(GDT_KTEXT)].m_limit1 = 0x7;
    x86_global_gdt[GDT_INDEX(GDT_KTEXT)].m_base0 = 0;
    x86_global_gdt[GDT_INDEX(GDT_KTEXT)].m_base1 = 0;
    x86_global_gdt[GDT_INDEX(GDT_KTEXT)].m_base2 = 0;
    x86_global_gdt[GDT_INDEX(GDT_KTEXT)].m_type = GDT_TYPE_CODE;
    x86_global_gdt[GDT_INDEX(GDT_KTEXT)].m_dpl = GDT_ACCESS_TYPE_SYSTEM;
    x86_global_gdt[GDT_INDEX(GDT_KTEXT)].m_present = 1;
    x86_global_gdt[GDT_INDEX(GDT_KTEXT)].m_32bits = 1;
    x86_global_gdt[GDT_INDEX(GDT_KTEXT)].m_granularity = 1;

    /*
     * TSS descriptor
     */
    ptr = &x86_global_tss;
    x86_global_gdt[GDT_INDEX(GDT_BOOT32)].m_limit0 = sizeof(x86_tss_t) - 1;
    x86_global_gdt[GDT_INDEX(GDT_BOOT32)].m_limit1 = 0;
    x86_global_gdt[GDT_INDEX(GDT_BOOT32)].m_base0 = (ulong_t) ptr & 0xFFFF;
    x86_global_gdt[GDT_INDEX(GDT_BOOT32)].m_base1 = ((ulong_t) ptr >> 16) & 0xFF;
    x86_global_gdt[GDT_INDEX(GDT_BOOT32)].m_base2 = ((ulong_t) ptr >> 24) & 0xFF;
    x86_global_gdt[GDT_INDEX(GDT_BOOT32)].m_type = GDT_TYPE_TSS;
    x86_global_gdt[GDT_INDEX(GDT_BOOT32)].m_dpl = GDT_ACCESS_TYPE_SYSTEM;
    x86_global_gdt[GDT_INDEX(GDT_BOOT32)].m_present = 1;
    x86_global_gdt[GDT_INDEX(GDT_BOOT32)].m_32bits = 0;
    x86_global_gdt[GDT_INDEX(GDT_BOOT32)].m_granularity = 0;

    /*
     * User data.  
     * 32 bits, offset by 2G
     */
    x86_global_gdt[GDT_INDEX(GDT_UDATA)].m_limit0 = 0xFFFF;
    x86_global_gdt[GDT_INDEX(GDT_UDATA)].m_limit1 = 0x7;
    x86_global_gdt[GDT_INDEX(GDT_UDATA)].m_base0 = 0;
    x86_global_gdt[GDT_INDEX(GDT_UDATA)].m_base1 = 0;
    x86_global_gdt[GDT_INDEX(GDT_UDATA)].m_base2 = 0x80;
    x86_global_gdt[GDT_INDEX(GDT_UDATA)].m_type = GDT_TYPE_DATAW;
    x86_global_gdt[GDT_INDEX(GDT_UDATA)].m_dpl = GDT_ACCESS_TYPE_USER;
    x86_global_gdt[GDT_INDEX(GDT_UDATA)].m_present = 1;
    x86_global_gdt[GDT_INDEX(GDT_UDATA)].m_32bits = 1;
    x86_global_gdt[GDT_INDEX(GDT_UDATA)].m_granularity = 1;

    /*
     * 32-bit user text
     */
    x86_global_gdt[GDT_INDEX(GDT_UTEXT)].m_limit0 = 0xFFFF;
    x86_global_gdt[GDT_INDEX(GDT_UTEXT)].m_limit1 = 0x7;
    x86_global_gdt[GDT_INDEX(GDT_UTEXT)].m_base0 = 0;
    x86_global_gdt[GDT_INDEX(GDT_UTEXT)].m_base1 = 0;
    x86_global_gdt[GDT_INDEX(GDT_UTEXT)].m_base2 = 0x80;
    x86_global_gdt[GDT_INDEX(GDT_UTEXT)].m_type = GDT_TYPE_CODER;
    x86_global_gdt[GDT_INDEX(GDT_UTEXT)].m_dpl = GDT_ACCESS_TYPE_USER;
    x86_global_gdt[GDT_INDEX(GDT_UTEXT)].m_present = 1;
    x86_global_gdt[GDT_INDEX(GDT_UTEXT)].m_32bits = 1;
    x86_global_gdt[GDT_INDEX(GDT_UTEXT)].m_granularity = 1;

    /*
     * Set GDT to our new one.
     */
    pd.m_pad = 0;
    pd.m_length = MAX_GDT * sizeof(x86_descriptor_t);
    pd.m_address = (ulong_t) & x86_global_gdt;
    lgdt(&pd);
}

/*
 * Initialize the memory and our GDT (different from the one used at
 * boot time) 
 */
static void x86_memory_initialize(void *hal, void *ptr)
{
    multiboot_information_t *mi = (multiboot_information_t *) ptr;
    kputs("Initializing memory...\n");

    /*
     * First load our new, non-boot GDT
     */
    kputs("   a. Load new GDT\n");
    x86_init_gdt();

    /* TODO: Allocate memory for kmalloc */
    kprintf("   Base %ldK Extended %ldK\n", mi->m_lower_memory,
	    mi->m_upper_memory);
}

/*
 * Shutdown the memory
 */
static void x86_memory_shutdown()
{
    /* Nothing to do here */
}

/*
 * External interface to the memory
 */
memory_device_t x86_memory_device = {
    x86_memory_initialize,
    x86_memory_shutdown,
};
