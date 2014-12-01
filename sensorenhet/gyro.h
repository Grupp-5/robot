/*
 * gyro.h
 *
 * Created: 11/25/2014 1:48:54 PM
 *  Author: emiva760, carth567
 */


#ifndef GYRO_H_
#define GYRO_H_

#include <stdint.h>

void gyro_init(void);

double current_degrees();
int16_t get_current_arate();

#endif /* GYRO_H_ */
