/*
 * gyro.h
 *
 * Created: 11/25/2014 1:48:54 PM
 *  Author: emiva760, carth567
 */


#ifndef GYRO_H_
#define GYRO_H_

/* Initiera ISP för överföring */
void SPI_init(void);

/* Hämta vinkelfrekvens */
int fetch_angular_rate(void);

#endif /* GYRO_H_ */
