#ifndef PRINT
#define PRINT

#include "keymain.h"

#ifdef DEBUG_PRINT_H
	#define DEBUG
#endif

#ifdef DEBUG

#include <stdio.h>
#include <string.h>
#include <avr/interrupt.h>

#define BAUD 57600
#define UBRR (((F_CPU / (BAUD * 16UL))) - 1) 

#define DEBUG_PRINT(arg) printf arg

static int
uart_putchar(char c, FILE *stream)
{
    if (c == '\n')
        uart_putchar('\r', stream);
    loop_until_bit_is_set(UCSRA, UDRE);
    UDR = c;
    return 0;
}

static FILE mystdout = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);

#else
#define DEBUG_PRINT(arg)
#endif

static void 
enable_printf(void)
{
#ifdef DEBUG
	stdout = &mystdout;
	//DDRD |= 0x01;

	UBRRH = (UBRR>>8);
	UBRRL = UBRR;
	UCSRA = 0x00;									   // asynchronous normal mode
	UCSRB = 0x08;									   // Tx enable, 8 data
	UCSRC = 0x86;									   // no parity, 1 stop, 8 data
#endif
}

#endif
