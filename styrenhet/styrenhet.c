/*
* styrenhet.c
*
* Created: 11/5/2014 4:57:27 PM
*  Author: dansj232, geoza435
*/

#include <avr/io.h>
#define F_CPU 16000000UL
#include "ax12.h" // TODO: Skriv en ReadAllServoInfo()
#include "servo.h"
#include "motion.h"
#include <util/delay.h>

int main(void)
{
	// Delay för att servona ska hinna starta, typ
	_delay_ms(10);
	
	uart_init();
	
	// För att manuellt avläsa data för debugging
	volatile ResponsePacket servo_infos[18];
	
	for (byte id = 1; id <= 18; id++) {
		servo_infos[id-1] = ReadAllAX(id);
	}
	
	SetSpeedAX(ID_BROADCAST, 200);
	SetTorqueAX(ID_BROADCAST, 300);
	TorqueEnableAX(ID_BROADCAST);
	
	setStartPosition();
	
	_delay_ms(4000);
	
	SetSpeedAX(ID_BROADCAST, 600);
	SetTorqueAX(ID_BROADCAST, 800);
	
	uint16_t wait_delay = 2000;
	uint16_t delay = 19;
	double from = 0;
	double to = 0.3;
	double step_size = 0.05;
	
	while(1) {
		_delay_ms(wait_delay);
		for (double step = from; step <= to; step += step_size) {
			tiltTo(step);
			_delay_ms(delay);
		}
		from = -to;
		
		_delay_ms(wait_delay);
		for (double step = to; step >= from; step -= step_size) {
			tiltTo(step);
			_delay_ms(delay);
		}
	}
}
