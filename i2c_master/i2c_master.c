/*
 * i2c_master.c
 *
 * Created: 11/14/2014 4:04:34 PM
 *  Author: emiva760, geoza435
 */

#define F_CPU 14745000UL // För test på komm-enheten

#include <util/twi.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include <util/delay.h>
#include <i2c.h>

Data dataToTransmit = {0};
Data dataToReceive = {0};

int main() {
	master_init(F_CPU, SCL_CLOCK);

	_delay_ms(200);

	DDRB = 0xFF;

	send_start_condition();

	dataToReceive.count = 4;
	master_receive(0x24, &dataToReceive);
	return;
}
