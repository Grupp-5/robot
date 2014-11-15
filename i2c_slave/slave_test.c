﻿/*
 * slave_test.c
 *
 * Created: 11/14/2014 12:50:03 PM
 *  Author: emiva760
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
		case REQUEST_TO_READ:
			slave_receive(&dataToReceive);
	}
	
	sei();
}

int main() {
	sei();
	
	slave_init(0x24);
	
	dataToTransmit.id = COMMUNICATION_DATA;
	dataToTransmit.count = 2;
	dataToTransmit.data[0] = 0b11010110;
	dataToTransmit.data[1] = 0b00101001;
	
	while (1) {}
}