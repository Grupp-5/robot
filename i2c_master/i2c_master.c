/*
 * i2c_master.c
 *
 * Created: 11/14/2014 4:04:34 PM
 *  Author: emiva760, geoza435, carth567
 */

#define F_CPU 8000000UL
//#define F_CPU 14745000UL // För test på komm-enheten

#include <util/twi.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include <util/delay.h>
#include <i2c.h>

Data dataToTransmit = {0};
Data dataToReceive = {0};

int main() {
	master_init(F_CPU, SCL_CLOCK);

	_delay_us(200);

	DDRB = 0xFF;
	PORTB &= 0;

	for (uint16_t i = 0; i < UINT16_MAX/10; i++) {
		dataToReceive.count = 4;
		master_receive(0x26, &dataToReceive);

		/*
		dataToTransmit.id = COMMUNICATION_DATA;
		dataToTransmit.count = 4;
		dataToTransmit.data[0] = 13;
		dataToTransmit.data[1] = 37;
		dataToTransmit.data[2] = 0;
		dataToTransmit.data[3] = 255;
		master_transmit(0x26, dataToTransmit);
		*/
		_delay_us(20);
	}
	PORTB |= (1<<PORTB0);
	_delay_us(100);

	while (1) {}
}
