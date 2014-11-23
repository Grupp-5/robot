/*
 * common.h
 *
 * Created: 11/20/2014 12:23:07 PM
 *  Author: emiva760, carth567
 */ 

#ifndef COMMON_H_
#define COMMON_H_

#include <avr/io.h>
#include <modulkom.h>

// Namn/adress på enheten.
typedef enum {
	DECISION = 0x20,
	CONTROL  = 0x30,
	SENSOR   = 0x40,
	COMMUNICATION = 0x50	// Alltid master
} Device_id;

typedef enum {
	SENSOR_DATA,
	COMMAND_DATA,
	P_DATA,
	D_DATA,
	PD_DATA //... OSV
} Data_id;

enum command {
	CHANGEMODE = 0x00,
	FORWARD = 0x01,
	BACK = 0x02,
	LEFT = 0x03,
	RIGHT = 0x04,
	STOP = 0x05,
	STOP_TIMER = 0x06,
	WAIT_FOR_P = 0x07,
	WAIT_FOR_D = 0x08,
	START_TIMER = 0x09
};



#endif /* COMMON_H_ */