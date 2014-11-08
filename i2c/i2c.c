/*
 * i2c.c
 *
 * Created: 2014-11-08 12:38:10
 *  Author: emiva760, carth567
 */ 


#include <avr/io.h>
#include <util/twi.h>
#include "i2c.h"

// Skicka data till I2C-enhet
unsigned char i2c_transmit(unsigned char type) {
	switch(type) {
	case I2C_START:	// Skicka start condition
		TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
		break;
	case I2C_DATA:	// Skicka data
		TWCR = (1 << TWINT) | (1 << TWEN);
		break;
	case I2C_STOP:	// Skicka stop condition
		TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);
		return 0;
	}
	
	// Vänta på att TWINT flaggan sätts i TWCR-register
	while(!(TWCR & (1 << TWINT)))
		;
	
	// Returnera TWI statusregister, maskera prescaler bitar (??)
	return (TWSR & TW_NO_INFO);
}

int main(void)
{
    while(1)
    {
        //TODO:: Please write your application code 
    }
}