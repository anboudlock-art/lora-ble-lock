#include "uart.h"
#include "stdio.h"

#ifdef  UART_DEBUG
int fputc(int ch, FILE *f)
{
	uart_0_write((unsigned char)ch);
	
	return ch;
}
#endif

