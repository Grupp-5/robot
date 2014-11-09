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
	
	// V�nta p� att TWINT flaggan s�tts i TWCR-register
	while(!(TWCR & (1 << TWINT)))
		;
	
	// Returnera TWI statusregister, maska prescaler bitar
	return (TWSR & 0xF8);
}

int i2c_writebyte(	unsigned int i2c_address, unsigned int dev_id, 
					unsigned int dev_addr, char data ) {
	unsigned char n = 0;
	unsigned char twi_status;
	char r_val = -1;
	
	i2c_retry:
	if(n++ >= MAX_TRIES) return r_val;
	
	twi_status = i2c_transmit(I2C_START);	// �verf�r Start condition
	
	// Kolla TWI-status
	if(twi_status == TW_MT_ARB_LOST) goto i2c_retry;
	if((twi_status != TW_START) && (twi_status != TW_REP_START))
		goto i2c_quit;
	
	// Skicka slavadress (SLA_W)
	TWDR = (dev_id & 0xF0) | ((dev_addr & 0x07) << 1) | TW_WRITE;
	
	// �verf�r i2c-data
	twi_status = i2c_transmit(I2C_DATA);
	
	// Kolla TWSR-status
	if((twi_status == TW_MT_SLA_NACK) || (twi_status == TW_MT_ARB_LOST))
		goto i2c_retry;
	if(twi_status != TW_MT_SLA_ACK) goto i2c_quit;
	
	// Skicka 8 mest signifikanta bitar av i2c-adressen
	TWDR = i2c_address >> 8;
	
	// �verf�r i2c-data
	twi_status = i2c_transmit(I2C_DATA);
	
	// Kolla TWSR-status
	if(twi_status != TW_MT_DATA_ACK) goto i2c_quit;
	
	// Skicka 8 LSB av i2c-adressen
	TWDR = i2c_address;
	
	// �verf�r i2c-data
	twi_status = i2c_transmit(I2C_DATA);
	
	// Kolla TWSR-status
	if(twi_status != TW_MT_DATA_ACK) goto i2c_quit;
	
	// L�gg data i dataregistret och p�b�rja �verf�ring
	TWDR = data;
	
	// �verf�r i2c-data
	twi_status = i2c_transmit(I2C_DATA);
	
	// Kolla TWSR-status
	if(twi_status != TW_MT_DATA_ACK) goto i2c_quit;
	
	// TWI-�verf�ring OK
	r_val = 1;
	
	i2_quit:
	// �verf�r i2c_data
	twi_status = i2c_transmit(I2C_STOP);
	return r_val;
}

int main(void)
{
    while(1)
    {
        //TODO:: Please write your application code 
    }
}