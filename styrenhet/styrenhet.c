/*
* styrenhet.c
*
* Created: 11/5/2014 4:57:27 PM
*  Author: dansj232, geoza435
*/

#include <avr/io.h>
#define F_CPU 16000000UL
#include "ax12.h"
#include <util/delay.h>

int main(void)
{
	uart_init();
	
	// Sparas undan hela tiden så att man kan läsa av i Debug
	ResponsePacket res;
	
	byte id = 12;
	
	res = PingAX(id);
	
	/* TODO: Sätts bara när servor startas om typ.
	Inte säker på vad som bör göras åt det. */
	
	res = SetSpeedAX(id, 300);
	res = SetTorqueAX(id, 150);
	res = TorqueEnableAX(id);
	while(1)
	{
		res = PingAX(id);
		
		// Tanken är att den ska stanna om den inte får något svar.
		if (res.error == 0xFE) {
			break;
		}
		
		res = SetPositionAX(id, 300);
		_delay_ms(1000);
		res = SetPositionAX(id, 400);
		_delay_ms(1000);
	}
}