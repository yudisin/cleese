extern void _VGA_Init(), _ISR_Init();

void
Cleese_Initialize()
{
	_VGA_Init();
	_ISR_Init();
}

void
Cleese_Finalize()
{
}
