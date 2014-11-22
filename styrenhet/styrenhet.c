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

	setLayPosition();
	setStartPosition();

	SetSpeedAX(ID_BROADCAST, 1000);
	SetTorqueAX(ID_BROADCAST, 800);

	uint16_t wait_delay = 10;
	double speed = 1;
	//double goal_step_length = 6;

	_delay_ms(3000);

	takeStep(speed, 0, 0, 0);
	while(1) {
		takeStep(speed, 0.5, 0, 0);
		_delay_ms(wait_delay);
	}

	/* // Gå fram och tillbaka
	// 0->goal_step_length
	for (double step_length = 0; step_length <= goal_step_length; step_length += 2) {
		_delay_ms(wait_delay);
		takeStep(step_size, step_length);
	}
	// Gå 5 steg
	for(uint8_t c = 0; c < 5; c++) {
		_delay_ms(wait_delay);
		takeStep(step_size, goal_step_length);
	}
	// Börja stanna
	for (double step_length = goal_step_length; step_length >= 0; step_length -= 2) {
		_delay_ms(wait_delay);
		takeStep(step_size, step_length);
	}
	for (double step_length = 0; step_length >= -goal_step_length; step_length -= 2) {
		_delay_ms(wait_delay);
		takeStep(step_size, step_length);
	}
	// gå fem steg baklänges
	for(uint8_t c = 0; c < 5; c++) {
		_delay_ms(wait_delay);
		takeStep(step_size, goal_step_length);
	}
	// börja stanna -goal_step_length -> 0
	for (double step_length = -goal_step_length; step_length <= 0; step_length += 2) {
		_delay_ms(wait_delay);
		takeStep(step_size, step_length);
	}
	*/
}
