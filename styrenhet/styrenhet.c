/*
* styrenhet.c
*
* Created: 11/5/2014 4:57:27 PM
*  Author: dansj232, geoza435
*/

#include <avr/io.h>
#define F_CPU 16000000UL
#include "servo.h"
#include <util/delay.h>

int main(void)
{
	uart_init();
	
	// Sparas undan hela tiden så att man kan läsa av i Debug
	ResponsePacket res;
	
	/* TODO: Sätts bara när servor startas om typ.
	Inte säker på vad som bör göras åt det. */
	
	res = SetSpeedAX(ID_BROADCAST, 100);
	res = SetTorqueAX(ID_BROADCAST, 1000);
	res = TorqueEnableAX(ID_BROADCAST);
	
	while(1)
	{
		res = SetSpeedAX(ID_BROADCAST, 30);
		
		SetLayPosition();
		_delay_ms(2000);
		
		res = SetSpeedAX(ID_BROADCAST, 200);
		
		SetStartPosition();
		
		res = ReadAX(11, AX_PRESENT_POSITION_L, 2);
		res = ReadAX(9, AX_PRESENT_POSITION_L, 2);
		
		_delay_ms(2000);
		
		Walk();
		_delay_ms(5000);
	}
}
