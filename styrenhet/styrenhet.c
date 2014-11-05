/*
* styrenhet.c
*
* Created: 11/5/2014 4:57:27 PM
*  Author: dansj232, geoza435
*/

#include <avr/io.h>
#include "ax12.h"

int main(void)
{
	uart_init();
	while(1)
	{
		PingAX(3);
	}
}