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
#include <util/atomic.h>
#include <common.h>
#include <modulkom.h>
#include <avr/interrupt.h>

Bus_data data_to_send = {0};
Bus_data data_to_receive = {0};

double forward_speed = 0;
double side_speed = 0;
double turn_speed = 0;

Bus_data prepare_data() {
	return data_to_send;
}

typedef union {
	Bus_data bus_data;
	struct {
		uint8_t count;
		data_id id;
		double forward_speed;
		double side_speed;
		double turn_speed;
	};
} Move_data;

void interpret_data(Bus_data data){
	data_to_receive = data;
	if(data_to_receive.id == MOVE) {
		Move_data move_data = (Move_data) data_to_receive;
		forward_speed = move_data.forward_speed;
		side_speed = move_data.side_speed;
		turn_speed = move_data.turn_speed;

		// Balla inte ur under debug, tack
		if(forward_speed > 1 || forward_speed < -1) {
			forward_speed = 0;
		}
		if(side_speed > 1 || side_speed < -1) {
			side_speed = 0;
		}
		if(turn_speed > 1 || forward_speed < -1) {
			turn_speed = 0;
		}
	}
}


int main(void)
{
	double forward_speed2 = 0;
	double side_speed2 = 0;
	double turn_speed2 = 0;
	set_as_slave(prepare_data, interpret_data, CONTROL);
	// Delay för att servona ska hinna starta, typ
	_delay_ms(10);
	
	uart_init();

	sei();
	
	// För att manuellt avläsa data för debugging
	volatile ResponsePacket servo_infos[18];
	
	for (byte id = 1; id <= 18; id++) {
		servo_infos[id-1] = ReadAllAX(id);
	}

	setLayPosition();
	setStartPosition();

	MCUSR = 0;

	SetSpeedAX(ID_BROADCAST, 1000);
	SetTorqueAX(ID_BROADCAST, 800);

	uint16_t wait_delay = 10;
	double speed = 0.7;
	//double goal_step_length = 6;

	_delay_ms(3000);

	takeStep(speed, 0, 0, 0);
	while(1) {
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
			forward_speed2 = forward_speed;
			side_speed2 = side_speed;
			turn_speed2 = turn_speed;
		}
		takeStep(speed, forward_speed2, side_speed2, turn_speed2);
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
