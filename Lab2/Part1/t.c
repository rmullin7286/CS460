/********************************************************************
Copyright 2010-2017 K.C. Wang, <kwang@eecs.wsu.edu>
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
********************************************************************/
#include <string.h>
#include <stdarg.h>
#include <stdint.h>

/*
UART0 base address: 0x101f1000;
UART1 base address: 0x101f2000;
UART2 base address: 0x101f3000;
UART3 base address: 0x10009000;

// flag register at 0x18
//  7    6    5    4    3    2   1   0
// TXFE RXFF TXFF RXFE BUSY
// TX FULL : 0x20
// TX empty: 0x80
// RX FULL : 0x40
// RX empty: 0x10
// BUSY=1 :  0x08
*/

int N;
int v[] = {1,2,3,4,5,6,7,8,9,10};
int sum;

char *tab = "0123456789ABCDEF";

//#include "string.c"
#include "uart.c"

void urpu(UART * up, uint32_t x, int base)
{
	static const char * ctable = "0123456789DEF";
	char c;
	if(x)
	{
		c = ctable[x % base];
		urpu(up, x / base, base);
		uputc(up, c);
	}
}

void uprintu_base(UART * up, uint32_t x, char * prefix, int base)
{
	uprints(up, prefix);
	(x == 0) ? uputc(up, '0') : urpu(up, x, base);
}

void uprintd(UART * up, int x)
{
	if(x < 0)
	{
		uputc(up, '-');
		x = -x;
	}
	uprintu_base(up, x, "", 10);
}

void uprintu(UART * up, uint32_t x)
{
	uprintu_base(up, x, "", 16);
}

void fuprintf(UART * up, char * fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	for(; *fmt != '\0'; fmt++)
	{
		if(*fmt == '%')
		{
			fmt++;
			switch(*fmt)
			{
				case 'c': uputc(up, va_arg(args, int));
					break;
				case 's': uprints(up, va_arg(args, char*));
					break;
				case 'u': uprintu(up, va_arg(args, uint32_t));
					break;
				case 'd': uprintd(up, va_arg(args, uint32_t));
			}
		}
	}
}

int main()
{
    int i;
    int size = sizeof(int);
    char string[32]; 
    char line[128];

    N = 10;

    uart_init();
  
	for(i = 0; i < 4; i++)
	{
		uprints(&uart[i], "enter a line from this UART : ");
		ugets(&uart[i], string);
		fuprintf(&uart[i], "    ECHO %s\n", string);
	}


    uprints(uart + 3, "Compute sum of array\n\r");
    sum = 0;
    for (i=0; i<N; i++)
		sum += v[i];
	fuprintf(uart + 3, "sum = %d\n", sum);
    
}
