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
 * thread.c
 *   Interface to the PC thread stuff
 */
#include <kstdlib.h>
#include <mach/x86/io.h>
#include <mach/x86/x86.h>
#include <mach/x86/thread.h>
#include <mach/x86/multiboot.h>

/*
 * We only use a single TSS because it's generally faster to use cr3
 * instead. 
 */
x86_tss_t x86_global_tss;

/*
 * Initialize and load our single, global TSS
 */
static void x86_thread_init_tss()
{
   /* TODO */
}

/*
 * Initialize the thread and TSS
 */
static void x86_thread_initialize(void *hal, void *ptr)
{
    //multiboot_information_t* mi = (multiboot_information_t*)ptr;
    kputs("Initializing threads...\n");
}

/*
 * Shutdown the thread
 */
static void x86_thread_shutdown()
{
    /* Nothing to do here */
}

/*
 * External interface to the thread
 */
thread_device_t x86_thread_device = {
    x86_thread_initialize,
    x86_thread_shutdown,
};
