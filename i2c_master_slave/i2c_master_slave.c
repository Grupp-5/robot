/*
 * i2c_master_slave.c
 *
 * Created: 11/17/2014 5:11:26 PM
 *  Author: geoza435
 */

#define F_CPU 14745000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <i2c.h>

#define THIS_ID 0x27
#define OTHER_ID 0x26

Data prepare_data() {
	Data dataToTransmit = {0};

	dataToTransmit.count = 4;
	dataToTransmit.data[0] = 13;
	dataToTransmit.data[1] = 37;
	dataToTransmit.data[2] = 0;
	dataToTransmit.data[3] = 255;

	return dataToTransmit;
}

void interpret_data(Data data) {
	if(data.data[0] != 48 || data.data[1] != 49 || data.count = 2) {
		PORTB |= (1 << PORTB2);
	}
}

int main(void) {
	/*********
	 * SLAVE *
	 *********/

	sei();
	slave_init(prepare_data, interpret_data, THIS_ID);


	/**********
	 * MASTER *
	 **********/

	Data dataToTransmit = {0};

	master_init(F_CPU, SCL_CLOCK);

	_delay_us(200);

	DDRB = 0xFF;
	PORTB &= 0;

	dataToTransmit.count = 2;
	dataToTransmit.data[0] = 48;
	dataToTransmit.data[1] = 49;

	Data dataToReceive = {0};
	dataToReceive.count = 4;

	while (1) {
		_delay_us(20);

		master_transmit(OTHER_ID, dataToTransmit);

		_delay_us(20);
		PORTB &= 0;
		_delay_us(1001);

		_delay_us(20);
		master_receive(OTHER_ID, &dataToReceive);

		_delay_us(20);
		PORTB &= 0;
		_delay_us(1001);
	}

	PORTB |= (1<<PORTB0);
	_delay_us(100);

	while (1) {}
}
