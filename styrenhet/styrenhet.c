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
double height = 0;
double xrot = 0;
double yrot = 0;
double speed = 1.5;

Bus_data prepare_data() {
	return data_to_send;
}

void interpret_data(Bus_data data){
	data_to_receive = data;
	if(data_to_receive.id == MOVE) {
		Move_data move_data = (Move_data) data_to_receive;
		forward_speed = move_data.forward_speed;
		side_speed = move_data.side_speed;
		turn_speed = move_data.turn_speed;

		// Balla inte ur under debug, tack
		if(forward_speed > 1 || forward_speed < -1) { forward_speed = 0; }
		if(side_speed > 1 || side_speed < -1) { side_speed = 0; }
		if(turn_speed > 1 || turn_speed < -1) { turn_speed = 0; }
	} else if (data_to_receive.id == SET_HEIGHT) {
		height = ((Constant_data) data_to_receive).constant;
		if (height > 1 || height < -1) { height = 0; }
	} else if (data_to_receive.id == ROTATION) {
		xrot = ((Rotate_data) data_to_receive).xrot;
		yrot = ((Rotate_data) data_to_receive).yrot;
		if (yrot > 1 || yrot < -1) { yrot = 0; }
		if (xrot > 1 || xrot < -1) { xrot = 0; }
	} else if (data_to_receive.id == SET_SPEED) {
		speed = ((Constant_data) data_to_receive).constant;
		if (speed > 4 || speed < 0) { speed = 0.5; }
	}
}


int main(void)
{
	volatile double forward_speed2 = 0;
	volatile double side_speed2 = 0;
	volatile double turn_speed2 = 0;
	volatile double height2 = 0;
	volatile double xrot2 = 0;
	volatile double yrot2 = 0;
	volatile double speed2 = 1.5;
	set_as_slave(F_CPU, prepare_data, interpret_data, CONTROL);
	// Delay för att servona ska hinna starta, typ
	_delay_ms(10);
	
	uart_init();

	sei();
	
	/*
	// För att manuellt avläsa data för debugging
	volatile ResponsePacket servo_infos[18];
	
	for (byte id = 1; id <= 18; id++) {
		servo_infos[id-1] = ReadAllAX(id);
	}
	*/

	//setLayPosition();
	setStartPosition();

	MCUSR = 0;

	SetSpeedAX(ID_BROADCAST, 1000);
	SetTorqueAX(ID_BROADCAST, 1000);

	_delay_ms(3000);

	takeStep(speed, 0, 0, 0, 0, 0, 0);
	while(1) {
		// Kanske inte behöver göra det här så ofta..
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
			forward_speed2 = forward_speed;
			side_speed2 = side_speed;
			turn_speed2 = turn_speed;
			height2 = height;
			xrot2 = xrot;
			yrot2 = yrot;
			speed2 = speed;
		}
		takeStep(speed2, forward_speed2, side_speed2, turn_speed2, height2, xrot2, yrot2);
	}
}
