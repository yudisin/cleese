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
 * cpu.c
 *   Interface to the PC cpu
 */
#include <kstdlib.h>
#include <cpu.h>
#include <mach/x86/cpu.h>
#include <mach/x86/multiboot.h>

static int x86_number_of_cpus = 1;
static cpu_descriptor_t x86_cpu_information[2] = {
    {
     VENDOR_INTEL,
     FAMILY_I686,
     0,
     },
    {
     VENDOR_INTEL,
     FAMILY_I686,
     0,
     },
};

/*
 * Initialize the cpu
 */
static void x86_cpu_initialize(void *hal, void *ptr)
{
    //multiboot_information_t* mi = (multiboot_information_t*)ptr;
    int i;
    kputs("Initializing CPU...\n");

    /* detect CPU's */
    for (i = 0; i < x86_number_of_cpus; i++) {
	kprintf("   Detected %s %s CPU\n",
		x86_cpu_information[i].m_vendor,
		x86_cpu_information[i].m_family);
    }
}

/*
 * Shutdown the cpu
 */
static void x86_cpu_shutdown()
{
    /* Nothing to do here */
}

/*
 * Beep at a given frequency
 */
static int x86_cpu_count(uint_t hz)
{
    /*
     * FIXME: We should sense the other CPU's on SMP machines
     */
    return (x86_number_of_cpus);
}

/*
 * Get information about a CPU
 */
static cpu_descriptor_t *x86_cpu_get_information(int i)
{
    /*
     * FIXME: We should have per-CPU information.
     */
    return (&x86_cpu_information[0]);
}

/*
 * Interface to the cpu
 */
cpu_device_t x86_cpu_device = {
    &x86_cpu_initialize,
    &x86_cpu_shutdown,
    &x86_cpu_count,
    &x86_cpu_get_information,
};
