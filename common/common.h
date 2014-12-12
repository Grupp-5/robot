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
	STOP_TIMER,
	SET_P,
	SET_D,
	SENSOR_DATA,
	SET_HEIGHT,
	ROTATION,
	PD_DATA,
	SET_SPEED
} Data_id;

// Hur mycket data ett paket har
extern uint8_t command_lengths[];

// [Kommando] | ska till | enhet
extern Device_id which_device[];

typedef union {
	Bus_data bus_data;
	struct {
		uint8_t count;
		data_id id;
		double forward_speed;
		double side_speed;
		double turn_speed;
	};
} Move_data;

typedef union {
	Bus_data bus_data;
	struct {
		uint8_t count;
		data_id id;
		double xrot;
		double yrot;
	};
} Rotate_data;

typedef union {
	Bus_data bus_data;
	struct {
		uint8_t count;
		data_id id;
		double fr;
		double br;
		double fl;
		double f;
		double bl;
		double gyro;
		int16_t arate;
	};
} Sensor_data;

typedef union {
	Bus_data bus_data;
	struct {
		uint8_t count;
		data_id id;
		double constant;
	};
} Constant_data;

typedef union {
	struct {
		uint8_t count;
		data_id id;
		double error;
		double p;
		double d;
		double adjustment;
	};
	Bus_data bus_data;
} PD_Data;

#endif /* COMMON_H_ */
