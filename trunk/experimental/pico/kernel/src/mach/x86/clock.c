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
 * clock.c
 *   Interface to the PC clock stuff
 */
#include <hal.h>
#include <kstdlib.h>
#include <mach/x86/io.h>
#include <mach/x86/atomic.h>
#include <mach/x86/clock.h>
#include <mach/x86/multiboot.h>

__volatile__ ulong_t x86_clock_ticks_counter;

/*
 * The ISR for handling clock ticks. Called about 20 times
 * a second
 */
static void x86_clock_isr(void *p)
{
    ATOMIC_INCL((void *) &x86_clock_ticks_counter);
}

/*
 * Initialize the clock and TSS
 */
static void x86_clock_initialize(void *h, void *ptr)
{
    hardware_abstraction_layer_t *hal = (hardware_abstraction_layer_t *) h;

    //multiboot_information_t* mi = (multiboot_information_t*)ptr;
    kputs("Initializing clock...\n");

    /*
     * The start of our clock
     */
    x86_clock_ticks_counter = 0;

    /*
     * Register the ISR. Note that this assumes the PIC stuff has been
     * initialized first.
     */
    hal->m_pic->register_handler(0, x86_clock_isr);
    hal->m_pic->enable(0);

    /*
     * initialise 8254 (or 8253) channel 0.  We set the timer to
     * generate an interrupt every time we have done enough ticks.
     * The output format is command, LSByte, MSByte.  Note that the
     * lowest the value of HZ can be is 19 - otherwise the
     * calculations get screwed up,
     */
    outportb(PIT_CTRL, CMD_SQR_WAVE);
    outportb(PIT_CH0, PIT_LATCH & 0x00ff);
    outportb(PIT_CH0, (PIT_LATCH & 0xff00) >> 8);
}


/*
 * Shutdown the clock
 */
static void x86_clock_shutdown()
{
    /* Nothing to do here */
}

/*
 * Get the number of ticks
 */
static ulong_t x86_clock_ticks()
{
    return (x86_clock_ticks_counter);
}

/*
 * External interface to the clock
 */
clock_device_t x86_clock_device = {
    x86_clock_initialize,
    x86_clock_shutdown,
    x86_clock_ticks,
};
