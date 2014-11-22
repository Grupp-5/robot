/*
 * common.h
 *
 * Created: 11/20/2014 12:23:07 PM
 *  Author: emiva760, carth567
 */ 

#ifndef COMMON_H_
#define COMMON_H_

#include <avr/io.h>

#define MAX_DATA 255 // maximalt antal data-bytes
#define SCL_CLOCK  100000L // "bitrate"

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

typedef struct {	// I2C-data. Ignore!
	uint8_t count;
	uint8_t data[MAX_DATA];
} Data;


typedef	struct {	// Data som ska skickas på bussen
	uint8_t count;
	Data_id id;
	uint8_t data[MAX_DATA - 1];
} Bus_data;


typedef union {
	Bus_data bus_data;
	Data data;
} Bus_data_union;



#endif /* COMMON_H_ */