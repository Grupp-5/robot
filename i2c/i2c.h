﻿/*
 * i2c.h
 *
 * Created: 11/11/2014 11:37:06 AM
 *  Author: emiva760
 */


#ifndef I2C_H_
#define I2C_H_

#define MAX_DATA 5 // maximalt antal data-bytes



#define REQUEST_TO_READ TW_SR_SLA_ACK
#define REQUEST_TO_WRITE TW_ST_SLA_ACK

typedef enum Id_t {

	/* 8-bit tal som motsvarar styrkommando och ev.
	ett 8-bit tal som motsvarar storlek och tecken */
	COMMUNICATION_DATA,

	/* 8-bit tal som motsvarar styrkommando och ev.
	ett 8-bit tal som motsvarar storlek och tecken */
	CONTROL_DATA,

	/* 8-bit tal som motsvarar styrkommando och ev.
	ett 8-bit tal som motsvarar storlek och tecken */
	DECISION_DATA,

	/* 2x8-bit avstånd (lång IR),  2x8-bit avstånd (medel IR),
	8-bit avstånd (kort IR) och 8-bit vinkel */
	SENSOR_DATA,
} Id;


typedef struct {
	Id id;
	int count;
	uint8_t data[MAX_DATA];
} Data;
void slave_init(uint8_t slave_address);

void send_start_condition(void);


void slave_transmit(Data data);
#endif /* I2C_H_ */
