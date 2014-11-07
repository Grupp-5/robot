/*
* styrenhet.c
*
* Created: 11/5/2014 4:57:27 PM
*  Author: dansj232, geoza435
*/

#include <avr/io.h>
#define F_CPU 8000000UL
#include "ax12.h"
#include <util/delay.h>

int main(void)
{
	uart_init();
	while(1)
	{
		PingAX(1);
		_delay_ms(50);
	}
}