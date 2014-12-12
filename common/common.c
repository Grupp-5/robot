/*
 * common.c
 *
 * Created: 12/11/2014 11:52:26 PM
 *  Author: geoza435
 */

#include "common.h"

uint8_t command_lengths[] = {
	[CHANGEMODE]  = 1,     // 0/1 Av/På
	[MOVE]        = 4+4+4, // step_forward + step_side + rotation
	[STOP_TIMER]  = 0,
	[SET_P]       = 4,
	[SET_D]       = 4,
	[SENSOR_DATA] = 4*6+2, // 5 IR-sensorer, 1 gyro och 1 angular rate
	[SET_HEIGHT]  = 4,     // 1 double
	[ROTATION]    = 4+4,
	[PD_DATA]     = 4*4,   // Error + P + D + Adjustment
	[SET_SPEED]   = 4
};

uint8_t which_device[] = {
	[CHANGEMODE]       = DECISION,
	[MOVE]             = CONTROL,
	[STOP_TIMER]       = COMMUNICATION,
	[SENSOR_DATA]      = CONTROL,
	[SET_P]            = DECISION,
	[SET_D]            = DECISION,
	[SET_HEIGHT]       = CONTROL,
	[ROTATION]         = CONTROL,
	[PD_DATA]          = COMMUNICATION,
	[SET_SPEED]        = CONTROL
};
