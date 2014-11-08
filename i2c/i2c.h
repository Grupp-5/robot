/*
 * i2c.h
 *
 * Created: 2014-11-08 22:01:05
 *  Author: emiva760, carth567
 */ 


#ifndef I2C_H_
#define I2C_H_

// Typ av sändning i i2c_transmit
#define I2C_START 0
#define I2C_DATA  1
#define I2C_STOP  2

unsigned char i2c_transmit(unsigned char type);
int i2c_writebyte(
					unsigned int i2c_address, unsigned int dev_id,
					unsigned int dev_addr, char data
				 );
int i2c_readbyte(
					unsigned int i2c_address, unsigned int dev_id,
					unsigned int dev_addr, char *data
				);

#endif /* I2C_H_ */