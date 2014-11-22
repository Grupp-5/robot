/*
 * slave_test.c
 *
 * Created: 11/14/2014 12:50:03 PM
 *  Author: emiva760, geoza435, carth567
 */
#include <util/twi.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#define F_CPU 14745600UL
#include <i2c.h>

#define THIS_ID 0x26

Data dataToReceive = {0};

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
	if(data.data[0] == 48 && data.data[1] == 49
		 && data.count == 2) {
		PORTB |= (1 << PORTB4);
	} else {
		PORTB |= (1 << PORTB2);
	}
}

int main() {
	PORTB = 0;
	sei();

	slave_init(prepare_data, interpret_data, THIS_ID);

	while (1) {}
}