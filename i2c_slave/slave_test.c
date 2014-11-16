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

Data dataToTransmit = {0};
Data dataToReceive = {0};

ISR(TWI_vect)
{
	cli();
	switch(TWSR)
	{
		case REQUEST_TO_WRITE:
			slave_transmit(dataToTransmit);
			break;
	}
	
	sei();
}

int main() {
	sei();
	
	slave_init(0x24);
	
	dataToTransmit.id = COMMUNICATION_DATA;
	dataToTransmit.count = 4;
	dataToTransmit.data[0] = 13;
	dataToTransmit.data[1] = 37;
	dataToTransmit.data[2] = 0;
	dataToTransmit.data[3] = 255;
	
	while (1) {}
}
