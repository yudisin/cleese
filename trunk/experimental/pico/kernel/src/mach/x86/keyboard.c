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
 * keyboard.c
 *   Interface to the PC keyboard
 */
#include <hal.h>
#include <kstdlib.h>
#include <mach/x86/io.h>
#include <mach/x86/pic.h>
#include <mach/x86/keyboard.h>
#include <mach/x86/multiboot.h>

#define MAX_DELAY 10000

hardware_abstraction_layer_t *hal = NULL;

/*
 * Wait for the keyboard to clear itself.
 */
void x86_keyboard_wait_until_done(void)
{
    uchar_t status;
    int i = 0;

    do {
	status = inportb(KBD_STATUS_PORT);
    } while (i++ < MAX_DELAY && (status & KBD_BUSY) == KBD_BUSY);
}

/*
 * Wait for some data to appear
 */
void x86_keyboard_wait_for_data(void)
{
    uchar_t status;
    int i = 0;

    do {
	status = inportb(KBD_STATUS_PORT);
    } while (i++ < MAX_DELAY && (status & KBD_DATA) == 0x00);
}

/*
 * Send a command to the keyboard
 */
void x86_keyboard_send_command(uchar_t cmd)
{
    x86_keyboard_wait_until_done();
    outportb(KBD_STATUS_PORT, cmd);
}

/*
 * Send some data to the keyboard
 */
void x86_keyboard_send_data(uchar_t data)
{
    x86_keyboard_wait_until_done();
    outportb(KBD_DATA_PORT, data);
}

/*
 * Get the available data from the keyboard
 */
uchar_t x86_keyboard_get_data(void)
{
    uchar_t val;

    x86_keyboard_wait_for_data();
    val = inportb(KBD_DATA_PORT);

    return val;
}

/*
 * The ISR for handling keys.
 */
static void x86_keyboard_isr(void *p)
{
    uchar_t scancode;
    uchar_t strobe;

    hal->m_pic->disable_interrupts();
    hal->m_pic->disable(1);

    scancode = x86_keyboard_get_data();

    /*
     * Tell the keyboard to accept more data
     */
    strobe = inportb(KBD_CONTROL_PORT);
    outportb(KBD_CONTROL_PORT, strobe | KBD_ENABLE);
    outportb(KBD_CONTROL_PORT, strobe);

    hal->m_pic->enable(1);
    hal->m_pic->enable_interrupts();
}

/*
 * Initialize the keyboard
 */
static void x86_keyboard_initialize(void *h, void *ptr)
{
    hal = (hardware_abstraction_layer_t *) h;

    kputs("Initializing keyboard...\n");

    /*
     * Register the ISR and enable the IRQ. Note that this assumes the 
     * PIC stuff has been initialized first.
     */
    hal->m_pic->register_handler(KBD_IRQ, x86_keyboard_isr);
    hal->m_pic->enable(KBD_IRQ);
}

/*
 * Shutdown the keyboard
 */
static void x86_keyboard_shutdown()
{
    /* Nothing to do here */
}

/*
 * Interface to the keyboard
 */
keyboard_device_t x86_keyboard_device = {
    x86_keyboard_initialize,
    x86_keyboard_shutdown,
};
