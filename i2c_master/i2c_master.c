/*
 * i2c_master.c
 *
 * Created: 11/14/2014 4:04:34 PM
 *  Author: emiva760, geoza435, carth567
 */

#define F_CPU 8000000UL
//#define F_CPU 14745000UL // För test på komm-enheten

#include <avr/io.h>
#include <avr/interrupt.h>

#include <util/delay.h>
#include <i2c.h>

#define OTHER_ID 0x27

int main() {
	Data dataToTransmit = {0};
	Data dataToReceive = {0};

	master_init(F_CPU, SCL_CLOCK);

	_delay_us(200);

	DDRB = 0xFF;
	PORTB &= 0;

	dataToTransmit.count = 2;
	dataToTransmit.data[0] = 48;
	dataToTransmit.data[1] = 49;

	dataToReceive.count = 4;

	while (1) {

		_delay_us(20);
		PORTB &= 0;
		_delay_us(20);

		master_transmit(OTHER_ID, dataToTransmit);

		_delay_us(20);
		PORTB &= 0;
		_delay_us(20);

		master_receive(OTHER_ID, &dataToReceive);
	}

	PORTB |= (1<<PORTB0);
	_delay_us(100);

	while (1) {}
}
