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
 * speaker.c
 *   Interface to the PC speaker
 */
#include <kstdlib.h>
#include <mach/x86/io.h>
#include <mach/x86/speaker.h>
#include <mach/x86/multiboot.h>

/*
 * Initialize the speaker
 */
static void x86_speaker_initialize(void *hal, void *ptr)
{
    //multiboot_information_t* mi = (multiboot_information_t*)ptr;
    kputs("Initializing speaker...\n");
}

/*
 * Shutdown the speaker
 */
static void x86_speaker_shutdown()
{
    /* Nothing to do here */
}

/*
 * Beep at a given frequency
 */
static void x86_speaker_beep(uint_t hz)
{
    outportb(X86_FREQUENCY_PORT, (uchar_t) hz);
    outportb(X86_FREQUENCY_PORT, (uchar_t) (hz >> 8));
    outportb(X86_SPEAKER_PORT, inportb(X86_SPEAKER_PORT) | 3);
}

/*
 * Turn off the speaker
 */
static void x86_speaker_stop()
{
    outportb(X86_SPEAKER_PORT, inportb(X86_SPEAKER_PORT) & !3);
}

/*
 * Interface to the speaker
 */
speaker_device_t x86_speaker_device = {
    x86_speaker_initialize,
    x86_speaker_shutdown,
    x86_speaker_beep,
    x86_speaker_stop,
};
