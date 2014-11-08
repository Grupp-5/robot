/*
* styrenhet.c
*
* Created: 11/5/2014 4:57:27 PM
*  Author: dansj232, geoza435
*/

#include <avr/io.h>
#define F_CPU 16000000UL
#include "ax12.h"

int main(void)
{
	uart_init();
	while(1)
	{
		byte id;
		
		for(id = 1; id <= 18; id++) {
			ResponsePacket res = PingAX(id);
		}
	}
}