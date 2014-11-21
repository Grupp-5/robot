/*
 * datatypes.h
 *
 * Created: 11/21/2014 5:05:26 AM
 *  Author: geoza435
 */


#ifndef DATATYPES_H_
#define DATATYPES_H_

#include <stdbool.h>
#include <stdint.h>

typedef uint8_t byte;

typedef struct {
	byte id;
	byte error;
	byte length;
	// TODO: 50 är kanske inte optimalt om det allokeras ofta
	byte params[50];
	byte checksum;
} ResponsePacket;


#endif /* DATATYPES_H_ */
