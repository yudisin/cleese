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
 */
#include <stdio.h>
#include <string.h>
#include <hal.h>

#ifndef BUFFSZ
#  define BUFFSZ 512
#endif

static char buffer[BUFFSZ];

/*
 * Send a formatted string to the physical console device.
 */
int kprintf(const char *fmt, ...)
{
    va_list args;
    int i;

    va_start(args, fmt);
    i = va_snprintf(buffer, BUFFSZ, fmt, args);
    va_end(args);

    HAL.m_console->puts(buffer);
    return (i);
}

/*
 * Send a string to the physical console device
 */
int kputs(const char *msg)
{
    HAL.m_console->puts(msg);
    return (strlen(msg));
}
