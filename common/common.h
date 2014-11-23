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
	CHANGEMODE,
	MOVE,
	START_TIMER,
	STOP_TIMER,
	SET_P,
	SET_D,
	SENSOR_DATA,
	SET_HEIGHT,
	ROTATION
	// o.s.v..
} Data_id;

// Hur mycket data ett paket har
static uint8_t command_lengths[] = {
	[CHANGEMODE]  = 1, // 0/1 Av/På
	[MOVE]        = 4+4+4, // step_forward + step_side + rotation
	                       // både doubles och floats är tydligen
	                       // 32 bitar stora.
	[START_TIMER] = 0,
	[STOP_TIMER]  = 0,
	[SET_P]       = 1, // 8-bitars-tal?
	[SET_D]       = 1,
	[SENSOR_DATA] = 2*5, // 5 doubles från sensorerna
	[SET_HEIGHT]  = 4, // 1 double
	[ROTATION]    = 4+4
};

// [Kommando] | ska till | enhet
static Device_id which_device[] = {
	[CHANGEMODE]       = DECISION,
	[MOVE]             = CONTROL,
	[START_TIMER]      = DECISION,
	[STOP_TIMER]       = DECISION,
	[SENSOR_DATA]      = CONTROL,
	[SET_P]            = DECISION,
	[SET_D]            = DECISION,
	[SET_HEIGHT]       = CONTROL,
	[ROTATION]         = CONTROL
};

#endif /* COMMON_H_ */