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
 * console.c
 */
#include <stdio.h>
#include <string.h>
#include <console.h>

#include <mach/x86/console.h>
#include <mach/x86/speaker.h>
#include <mach/x86/rtc.h>
#include <mach/x86/io.h>
#include <mach/x86/multiboot.h>

typedef struct {
    char *m_type;
    uchar_t m_rows;
    uchar_t m_columns;

    uchar_t m_tab_size;

    uchar_t m_cursor_x;
    uchar_t m_cursor_y;

    uchar_t m_scroll_start;
    uchar_t m_scroll_end;

    uchar_t m_attr;

    uint_t m_params[CONS_MAX_PARAMS];
    uint_t m_param_count;

    uint_t m_state;
    uint_t m_mode;

    ushort_t m_controller_port;
    ushort_t m_data_port;

    ushort_t *m_tvram;
} x86_console_t;

/*
 * Later turn this into something supporting vitual
 * consoles perhaps?
 */
static x86_console_t x86_default_console;
static x86_console_t *x86_current_console = &x86_default_console;

/*
 * x86_beep() 
 *   Turn on the speaker, then wait for a bit, and turn it off.
 */
static void x86_console_beep()
{
    int i;
    x86_speaker_device.beep(500);
    for (i = 0; i < 50000; i++);
    x86_speaker_device.stop();
}

static void x86_console_respond(const char *s)
{
    /* Do nothing */
}

/*
 * x86_set_cursor_pos()
 *   Set the real cursor position on the screen.
 */
static void x86_console_set_cursor_pos(uchar_t x, uchar_t y)
{
    uint_t x86_pos = (y * x86_current_console->m_columns) + x;

    outportb(x86_current_console->m_controller_port, 0xE);
    outportb(x86_current_console->m_data_port, (x86_pos >> 8) & 0xFF);
    outportb(x86_current_console->m_controller_port, 0xF);
    outportb(x86_current_console->m_data_port, x86_pos & 0xFF);
}

/*
 * x86_gotoxy()
 *   Set the virtual, and physical cursor position. 
 *
 * The position must be within the current scrolling area.
 */
static void x86_console_gotoxy(uchar_t x, uchar_t y)
{
    if (x >= x86_current_console->m_columns) {
	x = x86_current_console->m_columns - 1;
    }
    if (y < x86_current_console->m_scroll_start) {
	y = x86_current_console->m_scroll_start;
    }
    if (y >= x86_current_console->m_scroll_end) {
	y = x86_current_console->m_scroll_end - 1;
    }
    x86_current_console->m_cursor_x = x;
    x86_current_console->m_cursor_y = y;

    x86_console_set_cursor_pos(x, y);
}

/*
 * x86_cursor_on() 
 *     Turn the cursor on by moving it back on-screen.
 */
static void x86_console_cursor_on(void)
{
    x86_console_set_cursor_pos(x86_current_console->m_cursor_x,
			       x86_current_console->m_cursor_y);
}

/*
 * x86_cursor_off() 
 *     Turn the cursor off by moving it off-screen
 */
static void x86_console_cursor_off(void)
{
    x86_console_set_cursor_pos(x86_current_console->m_rows,
			       x86_current_console->m_columns + 1);
}

/*
 * x86_current_console->m_scroll_screen_up() 
 *     Scroll the screen up a number of lines
 */
static void x86_console_scroll_screen_up(uchar_t lines)
{
    ushort_t active_lines =
	x86_current_console->m_scroll_end -
	x86_current_console->m_scroll_start;
    uchar_t start = x86_current_console->m_scroll_start;
    ushort_t *ram_text =
	x86_current_console->m_tvram +
	(start * x86_current_console->m_columns);
    ushort_t fill = (x86_current_console->m_attr << 8) | ' ';

    /* Is it correct to set the attribute to the current attributes? */
    if (lines >= active_lines) {
	memsetw(ram_text, fill,
		active_lines * x86_current_console->m_columns);
    } else {
	memcpyw(ram_text,
		ram_text + (lines * x86_current_console->m_columns),
		(active_lines - lines) * x86_current_console->m_columns);
	memsetw(ram_text +
		((active_lines - lines) * x86_current_console->m_columns),
		fill, lines * x86_current_console->m_columns);
    }
}

/*
 * x86_current_console->m_scroll_screen_down() 
 *     Scroll the screen down a number of lines
 */
static void x86_console_scroll_screen_down(unsigned char lines)
{
    ushort_t active_lines =
	x86_current_console->m_scroll_end -
	x86_current_console->m_scroll_start;
    uchar_t start = x86_current_console->m_scroll_start;
    ushort_t *ram_text =
	x86_current_console->m_tvram +
	(start * x86_current_console->m_columns);
    ushort_t fill = (x86_current_console->m_attr << 8) | ' ';

    /* Is it correct to set the attribute to the current attributes? */
    if (lines >= active_lines) {
	memsetw(ram_text, fill,
		active_lines * x86_current_console->m_columns);
    } else {
	memrcpyw(ram_text + (lines * x86_current_console->m_columns),
		 ram_text,
		 (active_lines - lines) * x86_current_console->m_columns);
	memsetw(ram_text, fill, lines * x86_current_console->m_columns);
    }
}

/*
 * x86_console_move() 
 *     Move the cursor in a specified direction, a specified amount.
 *
 * If the cursor wanders outside the current scrolling area, and scrolling is
 * enabled, scroll the screen.
 */
static void
x86_console_move(uchar_t direction, uchar_t count,
		 uchar_t scrolling_allowed)
{
    uchar_t top = x86_current_console->m_scroll_start;
    uchar_t bottom = x86_current_console->m_scroll_end - 1;
    int cur_x = x86_current_console->m_cursor_x;
    int cur_y = x86_current_console->m_cursor_y;

    switch (direction) {
    case CONS_MOVE_UP:
	if (cur_y - count < top) {
	    if (scrolling_allowed) {
		x86_console_scroll_screen_down(top - (cur_y - count));
	    }
	    cur_y = top;
	} else {
	    cur_y -= count;
	}
	break;
    case CONS_MOVE_DOWN:
	if (cur_y + count > bottom) {
	    if (scrolling_allowed)
		x86_console_scroll_screen_up((cur_y + count) - bottom);
	    cur_y = bottom;
	} else {
	    cur_y += count;
	}
	break;
    case CONS_MOVE_LEFT:
	if (count > x86_current_console->m_columns) {
	    count = x86_current_console->m_columns;
	}
	if (cur_x - count < 0) {
	    cur_x = x86_current_console->m_columns + (cur_x - count);
	    if (cur_y > top) {
		cur_y -= 1;
	    } else {
		if (scrolling_allowed)
		    x86_console_scroll_screen_down(1);
		cur_y = top;
	    }
	} else {
	    cur_x -= count;
	}
	break;
    case CONS_MOVE_RIGHT:
	if (count > x86_current_console->m_columns) {
	    count = x86_current_console->m_columns;
	}
	if (cur_x + count > x86_current_console->m_columns) {
	    cur_x = x86_current_console->m_columns - (cur_x + count);
	    if (cur_y < bottom) {
		cur_y += 1;
	    } else {
		if (scrolling_allowed)
		    x86_console_scroll_screen_up(1);
		cur_y = bottom;
	    }
	} else {
	    cur_x += count;
	}
	break;
    }

    x86_current_console->m_cursor_x = cur_x;
    x86_current_console->m_cursor_y = cur_y;
}

/*
 * x86_console_write_char() 
 *     Output a single character to the actual device. 
 *
 * We don't worry about snow etc. Wrap when needed.
 */
static void x86_console_write_char(uchar_t c)
{
    ushort_t offset = 0;
    ushort_t code = (x86_current_console->m_attr << 8) | c;

    offset =
	(x86_current_console->m_cursor_y *
	 x86_current_console->m_columns) + x86_current_console->m_cursor_x;

    *(x86_current_console->m_tvram + offset) =
	(x86_current_console->m_attr << 8) | code;
    x86_current_console->m_cursor_x += 1;
    if (x86_current_console->m_cursor_x == x86_current_console->m_columns) {
	x86_current_console->m_cursor_x = 0;
	if (x86_current_console->m_cursor_y ==
	    x86_current_console->m_scroll_end - 1) {
	    x86_console_scroll_screen_up(1);
	} else {
	    x86_current_console->m_cursor_y += 1;
	}
    }
}

/*
 * x86_console_erase_to() 
 *     Erase part of the screen. 
 *
 * The parameter `where' decides where to erase to. This is relative to 
 * the current cursor position.
 */
static void x86_console_erase_to(uchar_t where)
{
    ushort_t start = 0;
    ushort_t length = 0;
    ushort_t fill = (x86_current_console->m_attr << 8) | ' ';

    switch (where) {
    case CONS_ERASE_EOS:
	start =
	    (x86_current_console->m_cursor_y *
	     x86_current_console->m_columns) +
	    x86_current_console->m_cursor_x;
	length =
	    (x86_current_console->m_rows *
	     x86_current_console->m_columns) - start;
	break;
    case CONS_ERASE_EOL:
	start =
	    (x86_current_console->m_cursor_y *
	     x86_current_console->m_columns) +
	    x86_current_console->m_cursor_x;
	length =
	    x86_current_console->m_columns -
	    x86_current_console->m_cursor_x;
	break;
    case CONS_ERASE_SOS:
	start = 0;
	length =
	    (x86_current_console->m_cursor_y *
	     x86_current_console->m_columns) +
	    x86_current_console->m_cursor_x + 1;
	break;
    case CONS_ERASE_SOL:
	start =
	    x86_current_console->m_cursor_y *
	    x86_current_console->m_columns;
	length = x86_current_console->m_cursor_x + 1;
	break;
    case CONS_ERASE_SCREEN:
	start =
	    x86_current_console->m_scroll_start *
	    x86_current_console->m_columns;
	length =
	    (x86_current_console->m_scroll_end -
	     x86_current_console->m_scroll_start) *
	    x86_current_console->m_columns;
	break;
    case CONS_ERASE_LINE:
	start =
	    x86_current_console->m_cursor_y *
	    x86_current_console->m_columns;
	length = x86_current_console->m_columns;
	break;
    }

    memsetw((x86_current_console->m_tvram + start), fill, length);
}

/*
 * x86_console_insert() 
 *     Insert some blank characters or lines.
 */
static void x86_console_insert(uint_t mode, uint_t num)
{
    ushort_t start;
    ushort_t length;
    ushort_t *ram_text = x86_current_console->m_tvram;
    ushort_t fill = (x86_current_console->m_attr << 8) | ' ';

    if (mode == CONS_LINE_MODE) {
	num *= x86_current_console->m_columns;
	start =
	    (x86_current_console->m_cursor_y +
	     1) * x86_current_console->m_columns;
    } else {
	start =
	    (x86_current_console->m_cursor_y *
	     x86_current_console->m_columns) +
	    x86_current_console->m_cursor_x + 1;
    }

    if (start + num >
	(x86_current_console->m_columns * x86_current_console->m_rows)) {
	length = start;
	memsetw(ram_text, fill, length);
	x86_console_gotoxy(0, 0);
	return;
    }
    ram_text += start;

    memrcpyw(ram_text + num, ram_text, num);
    memsetw(ram_text, fill, num);
}

/*
 * x86_console_delete() 
 *     Delete some characters or lines.
 */
static void x86_console_delete(uint_t mode, ushort_t num)
{
    ushort_t start;
    ushort_t length;
    ushort_t *ram_text = x86_current_console->m_tvram;
    ushort_t fill = (x86_current_console->m_attr << 8) | ' ';

    if (mode == CONS_LINE_MODE) {
	num *= x86_current_console->m_columns;
	start =
	    (x86_current_console->m_cursor_y +
	     1) * x86_current_console->m_columns;
    } else {
	start =
	    (x86_current_console->m_cursor_y *
	     x86_current_console->m_columns) +
	    x86_current_console->m_cursor_x + 1;
    }

    ram_text += start;

    if (start + num >
	(x86_current_console->m_columns * x86_current_console->m_rows)) {
	length =
	    (x86_current_console->m_columns *
	     x86_current_console->m_rows) - start;
	memsetw(ram_text, fill, length);
	return;
    }
    memcpyw(ram_text, ram_text + num,
	    (x86_current_console->m_columns *
	     x86_current_console->m_rows) - (start + num));

    ram_text -= start;
    ram_text +=
	(x86_current_console->m_columns * x86_current_console->m_rows) -
	num;

    memsetw(ram_text, fill, num);
}

/*
 * x86_console_set_attributes() 
 *     Set the current attribute byte from an array of parameters.
 */
static void x86_console_set_attributes(uint_t * attr, uchar_t num)
{
    int reverse = 0;
    int loop, a;

    x86_current_console->m_attr = CONS_WHITE;
    for (loop = 0; loop < num; loop++) {
	switch (attr[loop]) {
	case 0:		/* reset */
	    x86_current_console->m_attr = CONS_WHITE;
	    reverse = 0;
	    break;
	case 1:
	    x86_current_console->m_attr |= CONS_BOLD;
	    break;
	case 4:
	    /* can't do anything here */
	    break;
	case 5:
	    x86_current_console->m_attr |= CONS_BLINK;
	    break;
	case 7:		/* switch colors */
	    a = x86_current_console->m_attr & 0xF0;
	    x86_current_console->m_attr =
		(x86_current_console->m_attr << 4) | a;
	    reverse = 1;
	    break;
	case 8:
	    /* This should be secret */
	    x86_current_console->m_attr = CONS_BLACK;
	    break;
	case 30:
	    x86_current_console->m_attr =
		!reverse ? CONS_SET_FORE(x86_current_console->m_attr,
					 CONS_BLACK)
		: CONS_SET_BACK(x86_current_console->m_attr, CONS_BLACK);
	    break;
	case 31:
	    x86_current_console->m_attr =
		!reverse ? CONS_SET_FORE(x86_current_console->m_attr,
					 CONS_RED)
		: CONS_SET_BACK(x86_current_console->m_attr, CONS_RED);
	    break;
	case 32:
	    x86_current_console->m_attr =
		!reverse ? CONS_SET_FORE(x86_current_console->m_attr,
					 CONS_GREEN)
		: CONS_SET_BACK(x86_current_console->m_attr, CONS_GREEN);
	    break;
	case 33:
	    x86_current_console->m_attr =
		!reverse ? CONS_SET_FORE(x86_current_console->m_attr,
					 CONS_YELLOW)
		: CONS_SET_BACK(x86_current_console->m_attr, CONS_YELLOW);
	    break;
	case 34:
	    x86_current_console->m_attr =
		!reverse ? CONS_SET_FORE(x86_current_console->m_attr,
					 CONS_BLUE)
		: CONS_SET_BACK(x86_current_console->m_attr, CONS_BLUE);
	    break;
	case 35:
	    x86_current_console->m_attr =
		!reverse ? CONS_SET_FORE(x86_current_console->m_attr,
					 CONS_MAGENTA)
		: CONS_SET_BACK(x86_current_console->m_attr, CONS_MAGENTA);
	    break;
	case 36:
	    x86_current_console->m_attr =
		!reverse ? CONS_SET_FORE(x86_current_console->m_attr,
					 CONS_CYAN)
		: CONS_SET_BACK(x86_current_console->m_attr, CONS_CYAN);
	    break;
	case 37:
	    x86_current_console->m_attr =
		!reverse ? CONS_SET_FORE(x86_current_console->m_attr,
					 CONS_WHITE)
		: CONS_SET_BACK(x86_current_console->m_attr, CONS_WHITE);
	    break;
	case 40:
	    x86_current_console->m_attr =
		reverse ? CONS_SET_FORE(x86_current_console->m_attr,
					CONS_BLACK)
		: CONS_SET_BACK(x86_current_console->m_attr, CONS_BLACK);
	    break;
	case 41:
	    x86_current_console->m_attr =
		reverse ? CONS_SET_FORE(x86_current_console->m_attr,
					CONS_RED)
		: CONS_SET_BACK(x86_current_console->m_attr, CONS_RED);
	    break;
	case 42:
	    x86_current_console->m_attr =
		reverse ? CONS_SET_FORE(x86_current_console->m_attr,
					CONS_GREEN)
		: CONS_SET_BACK(x86_current_console->m_attr, CONS_GREEN);
	    break;
	case 43:
	    x86_current_console->m_attr =
		reverse ? CONS_SET_FORE(x86_current_console->m_attr,
					CONS_YELLOW)
		: CONS_SET_BACK(x86_current_console->m_attr, CONS_YELLOW);
	    break;
	case 44:
	    x86_current_console->m_attr =
		reverse ? CONS_SET_FORE(x86_current_console->m_attr,
					CONS_BLUE)
		: CONS_SET_BACK(x86_current_console->m_attr, CONS_BLUE);
	    break;
	case 45:
	    x86_current_console->m_attr =
		reverse ? CONS_SET_FORE(x86_current_console->m_attr,
					CONS_MAGENTA)
		: CONS_SET_BACK(x86_current_console->m_attr, CONS_MAGENTA);
	    break;
	case 46:
	    x86_current_console->m_attr =
		reverse ? CONS_SET_FORE(x86_current_console->m_attr,
					CONS_CYAN)
		: CONS_SET_BACK(x86_current_console->m_attr, CONS_CYAN);
	    break;
	case 47:
	    x86_current_console->m_attr =
		reverse ? CONS_SET_FORE(x86_current_console->m_attr,
					CONS_WHITE)
		: CONS_SET_BACK(x86_current_console->m_attr, CONS_WHITE);
	    break;
	}
    }
}

/*
 * x86_console_putchar() 
 *     Write a character to the screen and handle all the escape sequences. 
 * 
 * Most escape sequences are recognised, but some of the more obscure ones are
 * simply ignored.
 */
static void x86_console_putchar(char c)
{
    ushort_t foo, bar;
    char buff[64];

    switch (x86_current_console->m_state) {
    case CONS_STATE_NORMAL:
	switch (c) {
	case '\0':		/* This is a padding character    */
	    break;
	case '\7':		/* CTRL-G = beep                  */
	    x86_console_beep();
	    break;
	case '\b':		/* backspace character            */
	    if (x86_current_console->m_cursor_x > 0)
		x86_current_console->m_cursor_x -= 1;
	    break;
	case '\t':		/* tab character                  */
	    x86_console_putchar(' ');
	    while (x86_current_console->m_cursor_x %
		   x86_current_console->m_tab_size) {
		x86_console_putchar(' ');
	    }
	    break;
	case '\n':		/* line feed                      */
	    if (x86_current_console->m_cursor_y ==
		x86_current_console->m_scroll_end - 1) {
		x86_console_scroll_screen_up(1);
	    } else {
		x86_current_console->m_cursor_y += 1;
	    }
	    break;
	case '\r':		/* carriage return               */
	    x86_current_console->m_cursor_x = 0;
	    break;
	case '\33':		/* escape sequence starts        */
	    x86_current_console->m_state = CONS_STATE_ESCAPE;
	    break;
	default:
	    x86_console_write_char(c);	/* Filtering ??   */
	    break;
	}
	break;
    case CONS_STATE_ESCAPE:
	x86_current_console->m_state = CONS_STATE_NORMAL;
	switch (c) {
	case '(':
	    x86_current_console->m_state = CONS_STATE_ESCAPE_LPAREN;
	    break;
	case ')':
	    x86_current_console->m_state = CONS_STATE_ESCAPE_RPAREN;
	    break;
	case '#':
	    x86_current_console->m_state = CONS_STATE_ESCAPE_SHARP;
	    break;
	case '[':		/* start compound escape sequence */
	    x86_current_console->m_state = CONS_STATE_ESCAPE_BRACKET;
	    break;
	case '>':		/* numeric keypad mode            */
	    break;
	case '<':		/* VT52:  ansi mode               */
	    break;
	case '^':		/* VT52:  autoprint on            */
	    break;
	case ' ':		/* VT52:  autoprint off           */
	    break;
	case ']':		/* VT52:  print screen            */
	    break;
	case '=':		/* application keyboard mode      */
	    break;
	case '7':		/* save cursor and attribute      */
	    x86_console_cursor_off();	/* Correct ??     */
	    break;
	case '8':		/* restore cursor and attribute   */
	    x86_console_cursor_on();	/* Correct ??     */
	    break;
	case 'c':		/* reset everything               */
	    break;
	case 'A':		/* VT52:  cursor up               */
	    x86_console_move(CONS_MOVE_UP, 1, FALSE);
	    break;
	case 'B':		/* VT52:  cursor down             */
	    x86_console_move(CONS_MOVE_DOWN, 1, FALSE);
	    break;
	case 'C':		/* VT52:  cursor right            */
	    x86_console_move(CONS_MOVE_RIGHT, 1, FALSE);
	    break;
	case 'D':		/* Index. (VT52: cursor left)     */
	    if (x86_current_console->m_mode & CONS_VT52_PRIORITY) {
		x86_console_move(CONS_MOVE_LEFT, 1, FALSE);
	    } else {
		x86_console_move(CONS_MOVE_DOWN, 1, TRUE);
	    }
	    break;
	case 'E':		/* CR + Index                     */
	    x86_current_console->m_cursor_x = 0;
	    x86_console_move(CONS_MOVE_DOWN, 1, TRUE);
	    break;
	case 'F':		/* VT52:  graphics mode           */
	    x86_current_console->m_mode =
		CONS_VTGRAPH | CONS_VT52_PRIORITY;
	    break;
	case 'G':		/* VT52:  normal character mode   */
	    x86_current_console->m_mode = CONS_VT52_PRIORITY;
	    break;
	case 'H':		/* set horizontal tab (VT52:home) */
	    if (x86_current_console->m_mode & CONS_VT52_PRIORITY) {
		x86_console_gotoxy(0, 0);
	    }
	    break;
	case 'I':		/* VT52:  reverse index           */
	    x86_console_move(CONS_MOVE_UP, 1, FALSE);
	    break;
	case 'J':		/* VT52: erase to EOS             */
	    x86_console_erase_to(CONS_ERASE_EOS);
	    break;
	case 'K':		/* VT52: erase to EOL             */
	    x86_console_erase_to(CONS_ERASE_EOS);
	    break;
	case 'M':		/* reverse index                  */
	    x86_console_move(CONS_MOVE_UP, 1, TRUE);
	    break;
	case 'N':		/* set G2 for 1 character         */
	    break;
	case 'O':		/* set G3 for 1 character         */
	    break;
	case 'V':		/* VT52:  print cursor line       */
	    break;
	case 'W':		/* VT52:  print controller on     */
	    break;
	case 'X':		/* VT52:  print controller off    */
	    break;
	case 'Y':		/* VT52: position cursor          */
	    break;
	case 'Z':		/* VT52/VT102 term ident request  */
	    if (x86_current_console->m_mode & CONS_VT52_PRIORITY) {
		x86_console_respond(CONS_RESPOND_VT52_ID);
	    } else {
		x86_console_respond(CONS_RESPOND_VT102_ID);
	    }
	    break;
	}
	break;
    case CONS_STATE_ESCAPE_LPAREN:	/* ESC ( seen             */
	switch (c) {
	case 'A':		/* UK characters set as G0        */
	    break;
	case 'B':		/* USA characters set as G0       */
	    break;
	case '0':		/* Graphics as G0                 */
	    break;
	case '1':		/* Alt. ROM as G0                 */
	    break;
	case '2':		/* Special Alt. ROM as G0         */
	    break;
	}
	x86_current_console->m_state = CONS_STATE_NORMAL;
	break;
    case CONS_STATE_ESCAPE_RPAREN:	/* ESC ) seen             */
	switch (c) {
	case 'A':		/* UK characters set as G1        */
	    break;
	case 'B':		/* USA characters set as G1       */
	    break;
	case '0':		/* Graphics as G1                 */
	    break;
	case '1':		/* Alt. ROM as G1                 */
	    break;
	case '2':		/* Special Alt. ROM as G1         */
	    break;
	}
	x86_current_console->m_state = CONS_STATE_NORMAL;
	break;
    case CONS_STATE_ESCAPE_SHARP:	/* ESC # seen             */
	switch (c) {
	case '3':		/* top half of line attribute     */
	    break;
	case '4':		/* bottom half of line attribute  */
	    break;
	case '5':		/* single width & height          */
	    break;
	case '6':		/* double width                   */
	    break;
	case '8':		/* screen adjustment              */
	    break;
	}
	x86_current_console->m_state = CONS_STATE_NORMAL;
	break;
    case CONS_STATE_ESCAPE_BRACKET:	/* ESC [ seen             */
	for (foo = 0; foo < CONS_MAX_PARAMS; foo++) {
	    x86_current_console->m_params[foo] = 0;
	}
	x86_current_console->m_param_count = 0;
	x86_current_console->m_state = CONS_STATE_GET_PARAMETERS;
	/*
	 * Note that is is possible (and perhaps usual) for this to
	 * fall though to the next state. This is not a bug.
	 */
	if (c == '[') {
	    x86_current_console->m_state = CONS_STATE_FUNCTION_KEY;
	    break;
	}
	if (c == '?') {
	    break;
	}
    case CONS_STATE_GET_PARAMETERS:	/* ESC [ seen. Get params */
	if (c == ';'
	    && x86_current_console->m_param_count < CONS_MAX_PARAMS) {
	    x86_current_console->m_param_count += 1;
	    break;
	} else {
	    if (c >= '0' && c <= '9') {
		foo = x86_current_console->m_param_count;
		x86_current_console->m_params[foo] *= 10;
		x86_current_console->m_params[foo] += c - '0';
		break;
	    } else {
		x86_current_console->m_param_count += 1;
		x86_current_console->m_state = CONS_STATE_GOT_PARAMETERS;
	    }
	}
    case CONS_STATE_GOT_PARAMETERS:	/* ESC [ seen. Got params */
	x86_current_console->m_state = CONS_STATE_NORMAL;
	switch (c) {
	case '@':		/* insert N characters            */
	    if (x86_current_console->m_param_count >= 1) {
		foo = x86_current_console->m_params[0];
		x86_console_insert(CONS_CHAR_MODE, foo);
	    }
	    break;
	case 'c':		/* vt102 terminal identify request */
	    x86_console_respond(CONS_RESPOND_VT102_ID);
	    break;
	case 'f':		/* move cursor                     */
	    if (x86_current_console->m_param_count >= 2) {
		if (x86_current_console->m_params[1]) {
		    x86_current_console->m_params[1] -= 1;
		}
		if (x86_current_console->m_params[0]) {
		    x86_current_console->m_params[0] -= 1;
		}
		x86_console_gotoxy(x86_current_console->m_params[1],
				   x86_current_console->m_params[0]);
	    }
	    break;
	case 'g':		/* erase TABS                      */
	    break;
	case 'h':		/* set flags                       */
	    break;
	case 'i':		/* printer control                 */
	    break;
	case 'l':		/* reset flags                     */
	    break;
	case 'm':		/* set attributes                  */
	    if (x86_current_console->m_param_count >= 1) {
		x86_console_set_attributes(x86_current_console->m_params,
					   x86_current_console->
					   m_param_count);
	    }
	    break;
	case 'n':		/* report device status request    */
	    if (x86_current_console->m_param_count >= 1) {
		switch (x86_current_console->m_params[0]) {
		case 5:	/* status report request   */
		    x86_console_respond(CONS_RESPOND_STATUS_OK);
		    break;
		case 6:	/* cursor position request */
		    snprintf(buff, 64, "\033[%d;%dR",
			     x86_current_console->m_cursor_x,
			     x86_current_console->m_cursor_y);
		    x86_console_respond(buff);
		    break;
		case 15:	/* status request from
				 * printer 
				 */
		    x86_console_respond(CONS_RESPOND_STATUS_OK);
		    break;
		}
	    }
	    break;
	case 'q':		/* LED #1 on/off                   */
	    if (x86_current_console->m_params[0] == 0) {	/* off     */
	    } else {		/* on      */
	    }
	    break;
	case 'r':		/* set scrolling region            */
	    if (x86_current_console->m_param_count >= 2) {
		foo = x86_current_console->m_params[0];
		bar = x86_current_console->m_params[1];
		if (foo != bar) {
		    x86_current_console->m_scroll_start = MIN(foo, bar);
		    x86_current_console->m_scroll_end = MAX(foo, bar);
		}
	    }
	    break;
	case 'y':		/* invoke test                     */
	    x86_console_respond(CONS_RESPOND_STATUS_OK);
	    break;
	case 'A':		/* move up N lines                 */
	    if (x86_current_console->m_param_count >= 1) {
		x86_console_move(CONS_MOVE_UP,
				 x86_current_console->m_params[0], FALSE);
	    }
	    break;
	case 'B':		/* move down N lines               */
	    if (x86_current_console->m_param_count >= 1) {
		x86_console_move(CONS_MOVE_DOWN,
				 x86_current_console->m_params[0], FALSE);
	    }
	    break;
	case 'C':		/* move right N characters         */
	    if (x86_current_console->m_param_count >= 1) {
		x86_console_move(CONS_MOVE_RIGHT,
				 x86_current_console->m_params[0], FALSE);
	    }
	    break;
	case 'D':		/* move left M characters          */
	    if (x86_current_console->m_param_count >= 1) {
		x86_console_move(CONS_MOVE_LEFT,
				 x86_current_console->m_params[0], FALSE);
	    }
	    break;
	case 'H':		/* move cursor                     */
	    if (x86_current_console->m_param_count >= 2) {
		if (x86_current_console->m_params[1]) {
		    x86_current_console->m_params[1] -= 1;
		}
		if (x86_current_console->m_params[0]) {
		    x86_current_console->m_params[0] -= 1;
		}
		x86_console_gotoxy(x86_current_console->m_params[1],
				   x86_current_console->m_params[0]);
	    }
	    break;
	case 'J':		/* display oriented erase          */
	    if (x86_current_console->m_param_count >= 1) {
		switch (x86_current_console->m_params[0]) {
		case 0:
		    x86_console_erase_to(CONS_ERASE_EOS);
		    break;
		case 1:
		    x86_console_erase_to(CONS_ERASE_SOS);
		    break;
		case 2:
		    x86_console_erase_to(CONS_ERASE_SCREEN);
		    break;
		}
	    }
	    break;
	case 'K':		/* line oriented erase             */
	    if (x86_current_console->m_param_count >= 1) {
		switch (x86_current_console->m_params[0]) {
		case 0:
		    x86_console_erase_to(CONS_ERASE_EOL);
		    break;
		case 1:
		    x86_console_erase_to(CONS_ERASE_SOL);
		    break;
		case 2:
		    x86_console_erase_to(CONS_ERASE_LINE);
		    break;
		}
	    }
	    break;
	case 'L':		/* insert N lines                */
	    if (x86_current_console->m_param_count >= 1) {
		x86_console_insert(CONS_LINE_MODE,
				   x86_current_console->m_params[0]);
	    }
	    break;
	case 'M':		/* delete N lines                */
	    if (x86_current_console->m_param_count >= 1) {
		x86_console_delete(CONS_LINE_MODE,
				   x86_current_console->m_params[0]);
	    }
	    break;
	case 'P':		/* delete N characters to right  */
	    if (x86_current_console->m_param_count >= 1) {
		x86_console_delete(CONS_CHAR_MODE,
				   x86_current_console->m_params[0]);
	    }
	    break;
	}
	x86_current_console->m_state = CONS_STATE_NORMAL;
	break;
    case CONS_STATE_FUNCTION_KEY:	/* ESC [ [ seen          */
	x86_current_console->m_state = CONS_STATE_NORMAL;
	break;
    default:
	x86_current_console->m_state = CONS_STATE_NORMAL;
    }
}

/*
 * x86_console_puts
 *     Print a string on the screen. Map '\n' etc.
 */
void x86_console_puts(const char *s)
{
    while (*s) {
	if (*s == '\n')
	    x86_console_putchar('\r');
	x86_console_putchar(*s++);
    }
    x86_console_gotoxy(x86_current_console->m_cursor_x,
		       x86_current_console->m_cursor_y);
}

/*
 * x86_test() 
 *     A very simple test of the console driver. 
 *
 * It gives you something to look at while the system boots....
 */
static void x86_console_test_driver()
{
    unsigned int loop;
    char buff[64];

    x86_console_erase_to(CONS_ERASE_SCREEN);
    x86_console_gotoxy(0, 0);
    x86_console_puts
	("                                 CONSOLE TEST\n\n\n");
    x86_console_puts
	("          BLACK   RED     GREEN   YELLOW  BLUE    MAGENTA CYAN    WHITE\n");
    x86_console_puts("UNDERLINE ");
    for (loop = 30; loop < 38; loop++) {
	snprintf(buff, 64, "\033[%d;4m########", loop);
	x86_console_puts(buff);
    }
    x86_console_puts("\033[37m\n");
    x86_console_puts("BLINK     ");
    for (loop = 30; loop < 38; loop++) {
	snprintf(buff, 64, "\033[%d;5m########", loop);
	x86_console_puts(buff);
    }
    x86_console_puts("\033[37m\n");
    x86_console_puts("REVERSE   ");
    for (loop = 30; loop < 38; loop++) {
	snprintf(buff, 64, "\033[%d;7m########", loop);
	x86_console_puts(buff);
    }
    x86_console_puts("\033[37m\n");
    x86_console_puts("UN+BL+REV ");
    for (loop = 30; loop < 38; loop++) {
	snprintf(buff, 64, "\033[%d;4;5;7m########", loop);
	x86_console_puts(buff);
    }
    x86_console_puts("\033[37m\n");
}

/*
 * x86_initialise() 
 *     Initialise the display driver.
 *
 * Parse the arguments specifying the size and type of
 * display, and then set up the beep port, the card data and command ports,
 * the screen size, and the pointer to TVRAM.
 */
void x86_console_initialize(void *hal, void *ptr)
{
    //multiboot_information_t* mi = (multiboot_information_t*)ptr;
    int screen = rtcin(RTC_EQUIPMENT) & RTCEQB_DISP_MASK;

    /*
     * Basic initialization
     */
    x86_current_console->m_cursor_x = 0;	/* cursor positioning */
    x86_current_console->m_cursor_y = 0;
    x86_current_console->m_attr = 0x07;	/* this is plain white */
    x86_current_console->m_scroll_start = 0;	/* scrolling area */
    x86_current_console->m_scroll_end = 0;
    x86_current_console->m_param_count = 0;
    x86_current_console->m_state = CONS_STATE_NORMAL;
    x86_current_console->m_mode = CONS_MODE_NORMAL;
    x86_current_console->m_controller_port = 0;	/* controller port */
    x86_current_console->m_data_port = 0;	/* data port */
    x86_current_console->m_tab_size = CONS_TAB_SIZE;

    /*
     * Detect screen types and do the right thing
     */
    switch (screen) {
    case RTCEQB_DISP_CGA40:
	x86_current_console->m_type = "CGA/40";
	x86_current_console->m_tvram = (void *) CONS_COLOR_TVRAM;
	x86_current_console->m_rows = 40;
	x86_current_console->m_columns = 25;
	x86_current_console->m_controller_port = 0x3d4;
	x86_current_console->m_data_port = 0x3d5;
	break;

    case RTCEQB_DISP_CGA80:
	x86_current_console->m_type = "CGA/80";
	x86_current_console->m_tvram = (void *) CONS_COLOR_TVRAM;
	x86_current_console->m_rows = 80;
	x86_current_console->m_columns = 25;
	x86_current_console->m_controller_port = 0x3d4;
	x86_current_console->m_data_port = 0x3d5;
	break;

    case RTCEQB_DISP_MONO80:
	x86_current_console->m_type = "MONO";
	x86_current_console->m_tvram = (void *) CONS_MONO_TVRAM;
	x86_current_console->m_rows = 80;
	x86_current_console->m_columns = 25;
	x86_current_console->m_controller_port = 0x3b4;
	x86_current_console->m_data_port = 0x3b5;
	break;

    case RTCEQB_DISP_EGAVGA:
	x86_current_console->m_type = "EGA/VGA";
	x86_current_console->m_tvram = (void *) CONS_COLOR_TVRAM;
	x86_current_console->m_rows = 25;
	x86_current_console->m_columns = 80;
	x86_current_console->m_controller_port = 0x3d4;
	x86_current_console->m_data_port = 0x3d5;
	break;
    }

    x86_current_console->m_scroll_end = x86_current_console->m_rows;

    x86_console_erase_to(CONS_ERASE_SCREEN);
    x86_console_gotoxy(0, 0);
    x86_console_puts("Display type: ");
    x86_console_puts(x86_current_console->m_type);
    x86_console_puts("\n");

    //x86_console_test_driver ();
}

static void x86_console_shutdown()
{
    /* Nothing to do here today */
}

/*
 * Global console device interface
 */
console_device_t x86_console_device = {
    x86_console_initialize,
    x86_console_shutdown,
    x86_console_putchar,
    x86_console_puts,
};
