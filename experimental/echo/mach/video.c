extern unsigned char in(unsigned short);
extern void out(unsigned short, unsigned char);

#define VIDMEM              0xB8000

#define CRT_CTRL_PORT       0x3D4
#define CRT_DATA_PORT       0x3D5
#define CRT_CURSOR_LOC_HIGH 0x0E
#define CRT_CURSOR_LOC_LOW  0x0F

#define NUM_COLS 80
#define NUM_ROWS 25
#define SCREEN_SIZE (NUM_COLS * NUM_ROWS)

#define SCREEN_ATTR 0x0F

static int row, col;

void
clear_screen()
{
	unsigned char *vidmem = (unsigned char *) VIDMEM;
	int loop;

	for (loop = 0; loop < SCREEN_SIZE; loop++) {
		*vidmem++ = 0x20;
		*vidmem++ = SCREEN_ATTR;
	}
}

void
init_screen()
{
	row = col = 0;
	clear_screen();
}

static void
update_cursor()
{
	unsigned short offset;

	offset = (row * NUM_COLS) + col;

	out(CRT_CTRL_PORT, CRT_CURSOR_LOC_HIGH);
	out(CRT_DATA_PORT, (offset >> 8) & 0xFF);

	out(CRT_CTRL_PORT, CRT_CURSOR_LOC_LOW);
	out(CRT_DATA_PORT, offset & 0xFF);
}

static void
scroll_screen()
{
	unsigned short* v;
	int i;
	int n = SCREEN_SIZE;
	
	for (v = (unsigned short*) VIDMEM, i = 0; i < n; i++ ) {
		*v = *(v + NUM_COLS);
		++v;
	}

	for (v = (unsigned short*) VIDMEM + n, i = 0; i < NUM_COLS; i++) {
		*v++ = SCREEN_ATTR & (0x20 << 8);
	}
}

static void
new_line()
{
	++row;
	col = 0;
	if (row == NUM_ROWS) {
		scroll_screen();
		row = NUM_ROWS - 1;
	}
}

static void
clear_to_EOL()
{
	int loop;
	unsigned char *v = (unsigned char *) (VIDMEM + row * NUM_COLS * 2 + col * 2);

	for (loop = col; loop < NUM_COLS; loop++) {
		*v++ = ' ';
		*v++ = SCREEN_ATTR;
	}
}

void
print_char(int c)
{
	unsigned char *v = (unsigned char *) (VIDMEM + row * NUM_COLS * 2 + col * 2);

	switch(c) {
	case '\n':
		clear_to_EOL();
		new_line();
		break;
	default:
		*v++ = (unsigned char) c;
		*v   = SCREEN_ATTR;
		
		if (col < NUM_COLS - 1)
			++col;
		else
			new_line();
	}
}

void
print(const char *s)
{
	while (*s != '\0') {
		print_char(*s++);
	}
	update_cursor();
}

void
print_hex(unsigned long number)
{
	char buf[] = "0x00000000";
	char hex[] = "0123456789ABCDEF";
	unsigned int i, digit;

	for (i=9; i > 1; i--) {
		digit = number % 0x10;
		buf[i] = hex[digit];
		number /= 0x10;
		if (number == 0)
			break;
	}
	print(buf);
}
