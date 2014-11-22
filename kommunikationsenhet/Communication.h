/*
 * Communication.h
 *
 * Created: 2014-11-10 16:15:55
 *  Author: erima694 & eribo740
 */ 

#ifndef COMMUNICATION_H_
#define COMMUNICATION_H_

enum command {
	CHANGEMODE = 0x00,
	FORWARD = 0x01,
	BACK = 0x02,
	LEFT = 0x03,
	RIGHT = 0x04,
	STOP = 0x05,
	START_STOP_TIMER = 0x06,
	WAIT_FOR_P = 0x07,
	WAIT_FOR_D = 0x08
};

#endif /* COMMUNICATION_H_ */