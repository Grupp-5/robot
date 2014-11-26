/*
 * gyro.h
 *
 * Created: 11/25/2014 1:48:54 PM
 *  Author: emiva760, carth567
 */


#ifndef GYRO_H_
#define GYRO_H_

void SPI_init(void);
void activate_adc(void);
void deactivate_adc(void);
uint16_t read_ar_data(void);

#endif /* GYRO_H_ */
