/*
 * mach/x86/console.h
 *
 */
#ifndef __MACH_X86_CONSOLE_H__
#define __MACH_X86_CONSOLE_H__

#include <sys/types.h>
#include <console.h>

#define CONS_MAX_PARAMS 128
#define CONS_TAB_SIZE     8     /* Tab stops every 8 positions */

/*
 * Control ports
 */
#define CONS_CONTROLLER_PORT 0x3d4
#define CONS_MONO_DATA_PORT  0x3b5
#define CONS_COLOR_DATA_PORT 0x3d5
#define CONS_MONO_TVRAM      0xb0000
#define CONS_COLOR_TVRAM     0xb8000 /* EGA, VGA */

/*
 * Colors for the IBM et al.
 */
#define CONS_BLACK              0
#define CONS_BLUE               1
#define CONS_GREEN              2
#define CONS_CYAN               3
#define CONS_RED                4
#define CONS_MAGENTA            5
#define CONS_YELLOW             6
#define CONS_WHITE              7
#define CONS_BLINK             (1 << 8)
#define CONS_BOLD              (1 << 8)
#define CONS_FOREGROUND_MASK   0x0F
#define CONS_BACKGROUND_MASK   0xF0
#define CONS_SET_FORE(x,y) (((x) & ~CONS_FOREGROUND_MASK) | (y))
#define CONS_SET_BACK(x,y) (((x) & ~CONS_BACKGROUND_MASK) | ((y)<<4))

/*
 * These are used by con_move() to decide how to move the cursor
 */
#define CONS_MOVE_UP            1
#define CONS_MOVE_DOWN          2
#define CONS_MOVE_LEFT          3
#define CONS_MOVE_RIGHT         4

/*
 * These are used by con_erase() to decide where to clear to
 */
#define CONS_ERASE_EOS          1
#define CONS_ERASE_EOL          2
#define CONS_ERASE_SOS          3
#define CONS_ERASE_SOL          4
#define CONS_ERASE_SCREEN       5
#define CONS_ERASE_LINE         6

/*
 * These are used in con_insert() and con_delete()
 */
#define CONS_LINE_MODE          7
#define CONS_CHAR_MODE          8

/*
 * These are almost meaningless, but in the NEC version we have more states
 * for kanji mode etc.
 */
#define CONS_MODE_NORMAL         0
#define CONS_VTGRAPH       (1 << 1)     /* use the VT52 graphics set  */
#define CONS_VT52_PRIORITY (1 << 2)     /* interpret VT52 codes first     */

/*
 * These are the states of the escape sequence parser in con_putchar()
 */
#define CONS_STATE_NORMAL                0x00        /* Normal state             */
#define CONS_STATE_ESCAPE                0x01        /* ESC seen state           */
#define CONS_STATE_ESCAPE_LPAREN         0x02        /* ESC ( seen state         */
#define CONS_STATE_ESCAPE_RPAREN         0x03        /* ESC ) seen state         */
#define CONS_STATE_ESCAPE_BRACKET        0x04        /* ESC [ seen state         */
#define CONS_STATE_ESCAPE_SHARP          0x05        /* ESC # seen state         */
#define CONS_STATE_FUNCTION_KEY          0x06        /* ESC [[ seen state        */
#define CONS_STATE_GET_PARAMETERS        0x07        /* ESC [ seen. Read params  */
#define CONS_STATE_GOT_PARAMETERS        0x08        /* ESC [ seen. Got params   */

/*
 * These are the responses. Currently they are unused....
 */
#define CONS_RESPOND_VT52_ID     "\033/Z"
#define CONS_RESPOND_VT102_ID    "\033[?6c"
#define CONS_RESPOND_STATUS_OK   "\033[0n"

/*
 * One of these per kernel
 */
extern console_device_t x86_console_device;

#endif  /* __MACH_X86_CONSOLE_H__ */
