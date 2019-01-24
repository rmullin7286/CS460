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
#include <stdarg.h>
#include <stdint.h>

#define DR   0x00
#define FR   0x18

#define RXFE 0x10
#define TXFF 0x20

typedef struct uart{
  char *base;
  int n;
}UART;

UART uart[4];

int uart_init()
{
  int i; UART *up;

  for (i=0; i<4; i++){
    up = &uart[i];
    up->base = (char *)(0x101F1000 + i*0x1000);
    up->n = i;
  }
  uart[3].base = (char *)(0x10009000); // uart3 at 0x10009000
}

int uputc(UART *up, char c)
{
  while(*(up->base + FR) & TXFF);
  *(up->base + DR) = c;
}

int uprints(UART *up, char *s)
{
  while(*s)
    uputc(up, *s++);
}

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

void uprintf(UART * up, char * fmt, ...)
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
		else
			uputc(up, *fmt);
	}
}

int ugetc(UART *up)
{
  while (*(up->base + FR) & RXFE);
  return *(up->base + DR);
}



int ugets(UART *up, char *s)
{
  while ((*s = (char)ugetc(up)) != '\r'){
    uputc(up, *s);
    s++;
  }
 *s = 0;
}



/** WRITE YOUR uprintf(UART *up, char *fmt, . . .) for formatted print **/
