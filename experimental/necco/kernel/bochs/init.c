extern void _Ports_Init(), _VGA_Init(), _ISR_Init();

void
Cleese_Initialize()
{
	_Ports_Init();
	_VGA_Init();
	_ISR_Init();
	_Blit_Init();
}

void
Cleese_Finalize()
{
}
